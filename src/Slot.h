#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace morpher
{
    /** A fixed-size delay, one per corner, that lines each corner up with the
        slowest one (decision 0005). Allocated in prepare(); never allocates
        while processing.
    */
    class DelayLine
    {
    public:
        void prepare (int numChannels, int capacitySamples);
        void reset();
        int  getCapacity() const noexcept { return capacity; }

        /** Delays the first numChannels of the buffer in place, from start for num samples. */
        void process (juce::AudioBuffer<float>& buffer, int start, int num, int delaySamples) noexcept;

    private:
        juce::AudioBuffer<float> store;
        int capacity = 0, writePos = 0;
    };

    /** One corner of the pad: the plug-in in it, if any, and what it needs to
        run on the audio thread.

        Threads: only the message thread changes which plug-in is here, and
        only while holding the swap lock briefly. The audio thread only ever
        try-locks it, and if it can't (a swap is under way) the corner passes
        the sound through for that block rather than waiting.
    */
    class Slot
    {
    public:
        Slot() = default;

        //==========================================================================
        // Message thread.

        /** Allocates for a sample rate, block size and channel count, and
            prepares the plug-in here, if any, for them. Audio must not be
            running. */
        void prepare (double sampleRate, int maxBlockSize, int numChannels, bool nonRealtime);
        void release();

        /** Puts a plug-in here (or none) and hands back the one that was. The
            new one is prepared first, if audio is prepared. */
        std::unique_ptr<juce::AudioPluginInstance> swap (std::unique_ptr<juce::AudioPluginInstance>);

        juce::AudioPluginInstance* getInstance() const noexcept   { return instance.get(); }
        int  getPluginLatency() const;

        /** How long to hold this corner back so it lines up with the slowest. */
        void setDelay (int samples) noexcept      { delaySamples.store (samples); }
        int  getDelay() const noexcept            { return delaySamples.load(); }
        int  getDelayCapacity() const noexcept    { return delay.getCapacity(); }

        void setNonRealtime (bool) noexcept;
        void reset();

        //==========================================================================
        // Audio thread.

        /** Runs this corner over num samples of dry, from start, and writes
            its delayed output to out, from 0. An empty corner passes dry
            through (decision 0004). */
        void process (const juce::AudioBuffer<float>& dry, int start, int num, int numChannels,
                      const juce::MidiBuffer& midi, juce::AudioPlayHead* playHead,
                      juce::AudioBuffer<float>& out) noexcept;

        /** Prepares an instance to run with Morpher's layout. */
        static void configure (juce::AudioPluginInstance&, int numChannels,
                               double sampleRate, int maxBlockSize, bool nonRealtime);

    private:
        void allocateWork();

        juce::SpinLock swapLock;
        std::unique_ptr<juce::AudioPluginInstance> instance;
        juce::AudioBuffer<float> work;
        juce::MidiBuffer midiCopy;
        DelayLine delay;
        std::atomic<int> delaySamples { 0 };

        double sampleRate = 0.0;
        int maxBlock = 0, channels = 2;
        bool prepared = false, nonRealtime = false;

        JUCE_DECLARE_NON_COPYABLE (Slot)
    };
}
