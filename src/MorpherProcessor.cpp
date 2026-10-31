#include "MorpherProcessor.h"
#include "ui/MorpherEditor.h"

namespace morpher
{
    namespace
    {
        constexpr int stateVersion = 1;

        juce::String percentText (float v, int)
        {
            return juce::String (juce::roundToInt (v * 100.0f)) + "%";
        }

        float percentValue (const juce::String& text)
        {
            return juce::jlimit (0.0f, 1.0f, text.retainCharacters ("0123456789.").getFloatValue() / 100.0f);
        }

        juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
        {
            auto attributes = juce::AudioParameterFloatAttributes()
                                  .withStringFromValueFunction (percentText)
                                  .withValueFromStringFunction (percentValue);

            // Starts at corner A, so a plug-in loaded there is heard in full (decision 0006).
            return { std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "x", 1 }, "Morph X",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f, attributes),
                     std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "y", 1 }, "Morph Y",
                                                                  juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f, attributes) };
        }
    }

    //==========================================================================
    MorpherProcessor::MorpherProcessor()
        : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                           .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
          parameters (*this, nullptr, "PARAMETERS", createLayout())
    {
        xParam = parameters.getParameter ("x");
        yParam = parameters.getParameter ("y");
        xValue = parameters.getRawParameterValue ("x");
        yValue = parameters.getRawParameterValue ("y");
        shownX = xValue->load();
        shownY = yValue->load();

        juce::addDefaultFormatsToManager (formatManager);
    }

    MorpherProcessor::~MorpherProcessor()
    {
        cancelPendingUpdate();

        for (int s = 0; s < numSlots; ++s)
            if (auto old = slots[(size_t) s].swap (nullptr))
                old->removeListener (this);
    }

    //==========================================================================
    bool MorpherProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
    {
        const auto out = layouts.getMainOutputChannelSet();
        return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo())
                && layouts.getMainInputChannelSet() == out;
    }

    void MorpherProcessor::prepareToPlay (double sampleRate, int maxBlockSize)
    {
        const auto numCh = juce::jmax (1, getMainBusNumOutputChannels());
        preparedBlock = juce::jmax (1, maxBlockSize);

        dry    .setSize (numCh, preparedBlock);
        mixed  .setSize (numCh, preparedBlock);
        slotOut.setSize (numCh, preparedBlock);
        weights.allocate ((size_t) (numSlots * preparedBlock), true);
        chunkMidi.ensureSize (4096);
        bypassDelay.prepare (numCh, (int) std::ceil (sampleRate * 2.0) + preparedBlock);

        smoothX.reset (sampleRate, 0.02);
        smoothY.reset (sampleRate, 0.02);
        smoothX.setCurrentAndTargetValue (xValue->load());
        smoothY.setCurrentAndTargetValue (yValue->load());

        {
            const juce::ScopedLock sl (stateLock);

            for (auto& slot : slots)
                slot.prepare (sampleRate, preparedBlock, numCh, isNonRealtime());
        }

        updateLatency();
    }

    void MorpherProcessor::releaseResources()
    {
        const juce::ScopedLock sl (stateLock);

        for (auto& slot : slots)
            slot.release();
    }

    void MorpherProcessor::reset()
    {
        for (auto& slot : slots)
            slot.reset();

        bypassDelay.reset();
        smoothX.setCurrentAndTargetValue (xValue->load());
        smoothY.setCurrentAndTargetValue (yValue->load());
    }

    void MorpherProcessor::setNonRealtime (bool isNonRealtime) noexcept
    {
        AudioProcessor::setNonRealtime (isNonRealtime);

        for (auto& slot : slots)
            slot.setNonRealtime (isNonRealtime);
    }

    double MorpherProcessor::getTailLengthSeconds() const
    {
        // Some hosts ask from the audio thread, so this never waits on a lock.
        return tailSeconds.load();
    }

    //==========================================================================
    void MorpherProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
    {
        juce::ScopedNoDenormals noDenormals;

        const auto numCh = juce::jmin (getMainBusNumOutputChannels(), dry.getNumChannels(), buffer.getNumChannels());
        const auto total = buffer.getNumSamples();

        if (preparedBlock == 0 || numCh == 0)
            return;

        smoothX.setTargetValue (xValue->load (std::memory_order_relaxed));
        smoothY.setTargetValue (yValue->load (std::memory_order_relaxed));

        auto* host = getPlayHead();

        // A host may send more than it promised; take it in pieces we can hold.
        for (int start = 0; start < total; start += preparedBlock)
        {
            const auto num = juce::jmin (preparedBlock, total - start);
            const juce::MidiBuffer* chunk = &midi;

            if (total > preparedBlock)
            {
                chunkMidi.clear();
                chunkMidi.addEvents (midi, start, num, -start);
                chunk = &chunkMidi;
            }

            for (int ch = 0; ch < numCh; ++ch)
                dry.copyFrom (ch, 0, buffer, ch, start, num);

            for (int i = 0; i < num; ++i)
            {
                const auto w = weightsAt (smoothX.getNextValue(), smoothY.getNextValue());

                for (int s = 0; s < numSlots; ++s)
                    weights[s * preparedBlock + i] = w[(size_t) s];
            }

            mixed.clear();

            for (int s = 0; s < numSlots; ++s)
            {
                // Every corner runs all the time, heard or not, so its tail
                // and timing are right the moment you move towards it.
                slots[(size_t) s].process (dry, 0, num, numCh, *chunk, host, slotOut);

                const auto* w = weights.get() + s * preparedBlock;

                if (juce::FloatVectorOperations::findMaximum (w, num) <= 0.0f)
                    continue;

                for (int ch = 0; ch < numCh; ++ch)
                    juce::FloatVectorOperations::addWithMultiply (mixed.getWritePointer (ch),
                                                                  slotOut.getReadPointer (ch), w, num);
            }

            for (int ch = 0; ch < numCh; ++ch)
                buffer.copyFrom (ch, start, mixed, ch, 0, num);
        }

        for (int ch = numCh; ch < buffer.getNumChannels(); ++ch)
            buffer.clear (ch, 0, total);

        shownX.store (smoothX.getCurrentValue(), std::memory_order_relaxed);
        shownY.store (smoothY.getCurrentValue(), std::memory_order_relaxed);
    }

    void MorpherProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
    {
        // Bypassed, the dry sound keeps the same delay, so it stays in time.
        bypassDelay.process (buffer, 0, buffer.getNumSamples(), getLatencySamples());
    }

    //==========================================================================
    void MorpherProcessor::updateLatency()
    {
        int longest = 0;
        int longestSlot = -1;

        for (int s = 0; s < numSlots; ++s)
        {
            const auto l = slots[(size_t) s].getPluginLatency();

            if (l > longest)
            {
                longest = l;
                longestSlot = s;
            }
        }

        const auto capacity = slots[0].getDelayCapacity();

        if (capacity > 1 && longest > capacity - 1)
        {
            setNotice ({ getPluginName (longestSlot) + " in " + cornerName (longestSlot)
                           + " delays its sound by more than 2 seconds. Morpher can line up at most 2 seconds,"
                             " so the corners will be out of time.",
                         true, {}, {} });
            longest = capacity - 1;
        }

        double tail = 0.0;

        for (auto& slot : slots)
        {
            slot.setDelay (juce::jmax (0, longest - slot.getPluginLatency()));

            if (auto* p = slot.getInstance())
                tail = juce::jmax (tail, p->getTailLengthSeconds());
        }

        tailSeconds = tail;

        if (longest != getLatencySamples())
            setLatencySamples (longest);
    }

    void MorpherProcessor::audioProcessorChanged (AudioProcessor*, const juce::AudioProcessorListener::ChangeDetails& details)
    {
        if (details.latencyChanged)
            latencyDirty = true;

        if (details.nonParameterStateChanged || details.programChanged)
            stateDirty = true;

        triggerAsyncUpdate();
    }

    void MorpherProcessor::audioProcessorParameterChanged (AudioProcessor*, int, float)
    {
        // A hosted plug-in's settings changed, so the project has unsaved changes.
        if (! stateDirty.exchange (true))
            triggerAsyncUpdate();
    }

    void MorpherProcessor::handleAsyncUpdate()
    {
        std::unique_ptr<juce::XmlElement> toApply;

        {
            const juce::ScopedLock sl (stateLock);
            toApply = std::move (pendingState);
        }

        if (toApply != nullptr)
            applyState (*toApply);

        if (latencyDirty.exchange (false))
            updateLatency();

        if (stateDirty.exchange (false))
            updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withNonParameterStateChanged (true));
    }

    //==========================================================================
    MorpherProcessor::SlotStatus MorpherProcessor::getStatus (int s) const    { return info[(size_t) s].status; }
    juce::PluginDescription MorpherProcessor::getDescription (int s) const   { return info[(size_t) s].description; }
    juce::AudioPluginInstance* MorpherProcessor::getInstance (int s) const   { return slots[(size_t) s].getInstance(); }
    juce::String MorpherProcessor::getProblem (int s) const                  { return info[(size_t) s].problem; }

    juce::String MorpherProcessor::getPluginName (int s) const
    {
        if (! juce::isPositiveAndBelow (s, numSlots) || info[(size_t) s].status == SlotStatus::empty)
            return {};

        return info[(size_t) s].description.name;
    }

    juce::OwnedArray<juce::PluginDescription> MorpherProcessor::findPluginsIn (const juce::File& file)
    {
        juce::OwnedArray<juce::PluginDescription> found;
        const auto path = file.getFullPathName();

        for (auto* format : formatManager.getFormats())
            if (format->fileMightContainThisPluginType (path))
                format->findAllTypesForFile (found, path);

        return found;
    }

    juce::PluginDescription MorpherProcessor::locateMoved (const juce::PluginDescription& d) const
    {
        const juce::File original (d.fileOrIdentifier);

        if (! juce::File::isAbsolutePath (d.fileOrIdentifier) || original.exists())
            return d;

        // The plug-in may have moved, or the project come from another
        // computer: look for the same file in the usual places.
        for (auto* format : formatManager.getFormats())
        {
            if (format->getName() != d.pluginFormatName)
                continue;

            const auto search = format->getDefaultLocationsToSearch();

            for (int i = 0; i < search.getNumPaths(); ++i)
            {
                const auto dir = search[i];
                juce::Array<juce::File> candidates { dir.getChildFile (original.getFileName()) };

                for (const auto& sub : dir.findChildFiles (juce::File::findDirectories, false))
                    if (! sub.hasFileExtension (original.getFileExtension()))
                        candidates.add (sub.getChildFile (original.getFileName()));

                for (const auto& c : candidates)
                {
                    if (c.exists())
                    {
                        auto moved = d;
                        moved.fileOrIdentifier = c.getFullPathName();
                        return moved;
                    }
                }
            }
        }

        return d;
    }

    std::unique_ptr<juce::AudioPluginInstance> MorpherProcessor::takeOut (int s, std::unique_ptr<juce::AudioPluginInstance> next)
    {
        slotListeners.call ([s] (SlotListener& l) { l.slotWillChange (s); });

        if (next != nullptr)
            next->addListener (this);

        auto old = slots[(size_t) s].swap (std::move (next));

        if (old != nullptr)
            old->removeListener (this);

        return old;
    }

    juce::String MorpherProcessor::load (int s, const juce::PluginDescription& d, const juce::MemoryBlock* state)
    {
        JUCE_ASSERT_MESSAGE_THREAD
        juce::String error;

        {
            const juce::ScopedLock sl (stateLock);
            error = loadUnlocked (s, d, state, false);
        }

        userChangedSlots();
        return error;
    }

    juce::String MorpherProcessor::loadUnlocked (int s, const juce::PluginDescription& wanted,
                                                 const juce::MemoryBlock* state, bool keepIfMissing)
    {
        jassert (juce::isPositiveAndBelow (s, numSlots));
        auto& slotInfo = info[(size_t) s];

        // The same plug-in already here only needs its settings.
        if (auto* current = slots[(size_t) s].getInstance())
        {
            if (state != nullptr && slotInfo.description.isDuplicateOf (wanted))
            {
                current->setStateInformation (state->getData(), (int) state->getSize());
                return {};
            }
        }

        const auto d = locateMoved (wanted);
        const auto name = wanted.name.isNotEmpty() ? wanted.name : juce::File (wanted.fileOrIdentifier).getFileNameWithoutExtension();
        const auto rate  = getSampleRate() > 0.0 ? getSampleRate() : 44100.0;
        const auto block = getBlockSize() > 0 ? getBlockSize() : 512;

        juce::String error;
        std::unique_ptr<juce::AudioPluginInstance> instance;

        if (juce::File::isAbsolutePath (d.fileOrIdentifier) && ! juce::File (d.fileOrIdentifier).exists())
            error = "The file isn't there: " + d.fileOrIdentifier;
        else
            instance = formatManager.createPluginInstance (d, rate, block, error);

        if (instance == nullptr)
        {
            if (keepIfMissing)
            {
                takeOut (s, nullptr).reset();
                slotInfo.status      = SlotStatus::missing;
                slotInfo.description = wanted;
                slotInfo.keptState   = state != nullptr ? *state : juce::MemoryBlock();
                slotInfo.problem     = error;
                updateLatency();
            }

            return "Couldn't open " + name + ". Check that it's a 64-bit VST3 plug-in and that it opens on its own in your DAW.";
        }

        if (state != nullptr && state->getSize() > 0)
            instance->setStateInformation (state->getData(), (int) state->getSize());

        // Keep the description it was opened from: the instance's own names
        // the binary inside the bundle, which can't be opened again.
        auto loaded = d;
        if (loaded.name.isEmpty())
            loaded.name = instance->getName();

        takeOut (s, std::move (instance)).reset();

        slotInfo.status      = SlotStatus::loaded;
        slotInfo.description = loaded;
        slotInfo.keptState.reset();
        slotInfo.problem.clear();

        updateLatency();
        return {};
    }

    void MorpherProcessor::remove (int s)
    {
        JUCE_ASSERT_MESSAGE_THREAD

        {
            const juce::ScopedLock sl (stateLock);
            removeUnlocked (s);
        }

        userChangedSlots();
    }

    void MorpherProcessor::removeUnlocked (int s)
    {
        takeOut (s, nullptr).reset();
        info[(size_t) s] = {};
        updateLatency();
    }

    SlotContents MorpherProcessor::capture (int s) const
    {
        const juce::ScopedLock sl (stateLock);
        return captureUnlocked (s);
    }

    SlotContents MorpherProcessor::captureUnlocked (int s) const
    {
        SlotContents c;
        const auto& slotInfo = info[(size_t) s];

        if (slotInfo.status == SlotStatus::empty)
            return c;

        c.hasPlugin = true;
        c.description = slotInfo.description;

        if (auto* p = slots[(size_t) s].getInstance())
            p->getStateInformation (c.state);
        else
            c.state = slotInfo.keptState;

        return c;
    }

    void MorpherProcessor::restore (int s, const SlotContents& c)
    {
        JUCE_ASSERT_MESSAGE_THREAD

        {
            const juce::ScopedLock sl (stateLock);

            if (c.hasPlugin)
                loadUnlocked (s, c.description, &c.state, true);
            else
                removeUnlocked (s);
        }

        userChangedSlots();
    }

    juce::String MorpherProcessor::loadWithUndo (int s, const juce::PluginDescription& d)
    {
        const auto before = capture (s);
        const auto error = load (s, d, nullptr);

        if (error.isNotEmpty())
            setNotice ({ error, true, {}, {} });
        else if (before.hasPlugin)
            setNotice ({ "Replaced " + before.description.name + " in " + cornerName (s) + " with " + getPluginName (s) + ".",
                         false, "Undo", [this, s, before] { restore (s, before); } });

        return error;
    }

    void MorpherProcessor::removeWithUndo (int s)
    {
        const auto before = capture (s);

        if (! before.hasPlugin)
            return;

        remove (s);
        setNotice ({ "Removed " + before.description.name + " from " + cornerName (s) + ".",
                     false, "Undo", [this, s, before] { restore (s, before); } });
    }

    //==========================================================================
    void MorpherProcessor::setNotice (Notice n)
    {
        notice = std::move (n);
        changes.sendChangeMessage();
    }

    void MorpherProcessor::runNoticeAction()
    {
        // The action lives in the notice that dismissing clears, so take it out first.
        auto action = std::move (notice.action);
        dismissNotice();
        juce::NullCheckedInvocation::invoke (action);
    }

    void MorpherProcessor::dismissNotice()
    {
        notice = {};
        changes.sendChangeMessage();
    }

    void MorpherProcessor::slotsChanged()
    {
        changes.sendChangeMessage();
    }

    void MorpherProcessor::userChangedSlots()
    {
        // The project now has unsaved changes.
        updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withNonParameterStateChanged (true));
        slotsChanged();
    }

    //==========================================================================
    std::unique_ptr<juce::XmlElement> MorpherProcessor::createStateXml()
    {
        auto xml = std::make_unique<juce::XmlElement> ("MORPHER");
        xml->setAttribute ("version", stateVersion);

        if (auto params = parameters.copyState().createXml())
            xml->addChildElement (params.release());

        for (int s = 0; s < numSlots; ++s)
        {
            const auto c = captureUnlocked (s);

            if (! c.hasPlugin)
                continue;

            auto* slotXml = xml->createNewChildElement ("SLOT");
            slotXml->setAttribute ("index", s);
            slotXml->addChildElement (c.description.createXml().release());
            slotXml->createNewChildElement ("STATE")->addTextElement (c.state.toBase64Encoding());
        }

        return xml;
    }

    void MorpherProcessor::getStateInformation (juce::MemoryBlock& dest)
    {
        std::unique_ptr<juce::XmlElement> xml;

        {
            const juce::ScopedLock sl (stateLock);

            // A state not yet applied is still the state: never save over it with less.
            xml = pendingState != nullptr ? std::make_unique<juce::XmlElement> (*pendingState)
                                          : createStateXml();
        }

        copyXmlToBinary (*xml, dest);
    }

    void MorpherProcessor::setStateInformation (const void* data, int size)
    {
        auto xml = getXmlFromBinary (data, size);

        if (xml == nullptr || ! xml->hasTagName ("MORPHER"))
            return;

        if (auto* mm = juce::MessageManager::getInstanceWithoutCreating(); mm != nullptr && mm->isThisTheMessageThread())
        {
            {
                const juce::ScopedLock sl (stateLock);
                pendingState.reset();
            }

            applyState (*xml);
            return;
        }

        // Plug-ins can only be opened on the message thread.
        {
            const juce::ScopedLock sl (stateLock);
            pendingState = std::move (xml);
        }

        triggerAsyncUpdate();
    }

    void MorpherProcessor::applyState (const juce::XmlElement& xml)
    {
        JUCE_ASSERT_MESSAGE_THREAD

        if (auto* params = xml.getChildByName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*params));

        juce::StringArray missing;

        {
            const juce::ScopedLock sl (stateLock);

            for (int s = 0; s < numSlots; ++s)
            {
                const juce::XmlElement* slotXml = nullptr;

                for (auto* child : xml.getChildWithTagNameIterator ("SLOT"))
                    if (child->getIntAttribute ("index", -1) == s)
                        slotXml = child;

                const auto* pluginXml = slotXml != nullptr ? slotXml->getChildByName ("PLUGIN") : nullptr;
                juce::PluginDescription d;

                if (pluginXml == nullptr || ! d.loadFromXml (*pluginXml))
                {
                    if (info[(size_t) s].status != SlotStatus::empty)
                        removeUnlocked (s);

                    continue;
                }

                juce::MemoryBlock state;
                state.fromBase64Encoding (slotXml->getChildElementAllSubText ("STATE", {}));

                if (loadUnlocked (s, d, &state, true).isNotEmpty())
                    missing.add (d.name + " (" + cornerName (s) + ")");
            }
        }

        if (! missing.isEmpty())
            setNotice ({ "Couldn't open " + missing.joinIntoString (", ") + ". "
                           + (missing.size() == 1 ? "Its settings are kept and come back when it's installed,"
                                                    " or choose another plug-in in its place."
                                                  : "Their settings are kept and come back when they're installed,"
                                                    " or choose other plug-ins in their place."),
                         true, {}, {} });

        slotsChanged();
    }

    //==========================================================================
    juce::AudioProcessorEditor* MorpherProcessor::createEditor()
    {
        return new MorpherEditor (*this);
    }
}
