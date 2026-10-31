#include "Slot.h"

namespace morpher
{
    //==========================================================================
    void DelayLine::prepare (int numChannels, int capacitySamples)
    {
        capacity = juce::jmax (1, capacitySamples);
        store.setSize (juce::jmax (1, numChannels), capacity);
        reset();
    }

    void DelayLine::reset()
    {
        store.clear();
        writePos = 0;
    }

    void DelayLine::process (juce::AudioBuffer<float>& buffer, int start, int num, int delaySamples) noexcept
    {
        const auto d = juce::jlimit (0, capacity - 1, delaySamples);
        const auto numCh = juce::jmin (buffer.getNumChannels(), store.getNumChannels());
        int endPos = writePos;

        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* data = buffer.getWritePointer (ch, start);
            auto* line = store.getWritePointer (ch);
            int w = writePos;

            for (int i = 0; i < num; ++i)
            {
                line[w] = data[i];
                auto r = w - d;
                if (r < 0) r += capacity;
                data[i] = line[r];
                if (++w == capacity) w = 0;
            }

            endPos = w;
        }

        writePos = endPos;
    }

    //==========================================================================
    void Slot::configure (juce::AudioPluginInstance& p, int numChannels,
                          double rate, int blockSize, bool isNonRealtime)
    {
        // Ask for Morpher's own layout on the main buses. A plug-in that
        // won't take it keeps its own, and process() maps between the two.
        auto layout = p.getBusesLayout();
        const auto set = juce::AudioChannelSet::canonicalChannelSet (numChannels);

        if (! layout.inputBuses.isEmpty())   layout.inputBuses.getReference (0) = set;
        if (! layout.outputBuses.isEmpty())  layout.outputBuses.getReference (0) = set;

        if (layout != p.getBusesLayout() && p.checkBusesLayoutSupported (layout))
            p.setBusesLayout (layout);

        p.setProcessingPrecision (juce::AudioProcessor::singlePrecision);
        p.setRateAndBufferSizeDetails (rate, blockSize);
        p.setNonRealtime (isNonRealtime);
        p.prepareToPlay (rate, blockSize);
    }

    void Slot::prepare (double rate, int maxBlockSize, int numChannels, bool isNonRealtime)
    {
        const juce::SpinLock::ScopedLockType sl (swapLock);

        if (prepared && instance != nullptr)
            instance->releaseResources();

        sampleRate  = rate;
        maxBlock    = juce::jmax (1, maxBlockSize);
        channels    = juce::jmax (1, numChannels);
        nonRealtime = isNonRealtime;

        // Two seconds lines up any plug-in we know of, linear-phase EQs included.
        delay.prepare (channels, (int) std::ceil (rate * 2.0) + maxBlock);
        midiCopy.ensureSize (4096);

        if (instance != nullptr)
            configure (*instance, channels, sampleRate, maxBlock, nonRealtime);

        allocateWork();
        prepared = true;
    }

    void Slot::release()
    {
        const juce::SpinLock::ScopedLockType sl (swapLock);

        if (prepared && instance != nullptr)
            instance->releaseResources();

        prepared = false;
    }

    void Slot::allocateWork()
    {
        auto numCh = channels;

        if (instance != nullptr)
            numCh = juce::jmax (numCh, instance->getTotalNumInputChannels(), instance->getTotalNumOutputChannels());

        work.setSize (numCh, maxBlock, false, true, false);
    }

    std::unique_ptr<juce::AudioPluginInstance> Slot::swap (std::unique_ptr<juce::AudioPluginInstance> next)
    {
        // Everything that allocates happens here, before the lock.
        juce::AudioBuffer<float> nextWork;

        if (prepared)
        {
            if (next != nullptr)
                configure (*next, channels, sampleRate, maxBlock, nonRealtime);

            auto numCh = channels;

            if (next != nullptr)
                numCh = juce::jmax (numCh, next->getTotalNumInputChannels(), next->getTotalNumOutputChannels());

            nextWork.setSize (numCh, maxBlock);
        }

        {
            const juce::SpinLock::ScopedLockType sl (swapLock);
            std::swap (instance, next);

            if (prepared)
                std::swap (work, nextWork);

            delay.reset();
        }

        if (next != nullptr && prepared)
            next->releaseResources();

        return next;
    }

    int Slot::getPluginLatency() const
    {
        return instance != nullptr ? juce::jmax (0, instance->getLatencySamples()) : 0;
    }

    void Slot::setNonRealtime (bool isNonRealtime) noexcept
    {
        nonRealtime = isNonRealtime;

        if (instance != nullptr)
            instance->setNonRealtime (isNonRealtime);
    }

    void Slot::reset()
    {
        const juce::SpinLock::ScopedLockType sl (swapLock);

        if (instance != nullptr)
            instance->reset();

        delay.reset();
    }

    //==========================================================================
    void Slot::process (const juce::AudioBuffer<float>& dry, int start, int num, int numChannels,
                        const juce::MidiBuffer& midi, juce::AudioPlayHead* playHead,
                        juce::AudioBuffer<float>& out) noexcept
    {
        jassert (num <= maxBlock && out.getNumSamples() >= num);

        const juce::SpinLock::ScopedTryLockType sl (swapLock);
        auto* plugin = sl.isLocked() ? instance.get() : nullptr;

        const auto mainIns  = plugin != nullptr ? plugin->getMainBusNumInputChannels()  : 0;
        const auto mainOuts = plugin != nullptr ? plugin->getMainBusNumOutputChannels() : 0;

        if (plugin == nullptr || mainOuts == 0)
        {
            // Empty, mid-swap, or a plug-in that makes no sound: pass the sound through.
            for (int ch = 0; ch < numChannels; ++ch)
                out.copyFrom (ch, 0, dry, ch, start, num);
        }
        else
        {
            juce::AudioBuffer<float> view (work.getArrayOfWritePointers(), work.getNumChannels(), num);

            for (int ch = 0; ch < view.getNumChannels(); ++ch)
            {
                if (ch >= mainIns)
                    view.clear (ch, 0, num);
                else if (mainIns == 1 && numChannels > 1)
                {
                    // A mono plug-in in a stereo place hears both sides.
                    view.clear (ch, 0, num);
                    view.addFrom (ch, 0, dry, 0, start, num, 0.5f);
                    view.addFrom (ch, 0, dry, 1, start, num, 0.5f);
                }
                else
                    view.copyFrom (ch, 0, dry, juce::jmin (ch, numChannels - 1), start, num);
            }

            midiCopy.clear();
            midiCopy.addEvents (midi, 0, num, 0);

            plugin->setPlayHead (playHead);
            plugin->processBlock (view, midiCopy);

            for (int ch = 0; ch < numChannels; ++ch)
                out.copyFrom (ch, 0, view, juce::jmin (ch, mainOuts - 1), 0, num);
        }

        if (sl.isLocked())
            delay.process (out, 0, num, delaySamples.load (std::memory_order_relaxed));
    }
}
