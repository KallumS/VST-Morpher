#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "MorphMath.h"
#include "Slot.h"

namespace morpher
{
    /** A message for the line at the top of the window. It stays until
        dismissed or replaced (Apple: avoid UI that times out), and it lives
        here rather than in the window so closing the window doesn't lose it.
    */
    struct Notice
    {
        juce::String text;
        bool isWarning = false;
        juce::String actionName;          // "Undo", or empty for none
        std::function<void()> action;
    };

    /** What a corner holds, as far as saving and Undo are concerned. */
    struct SlotContents
    {
        bool hasPlugin = false;
        juce::PluginDescription description;
        juce::MemoryBlock state;
    };

    class MorpherProcessor final : public juce::AudioProcessor,
                                   private juce::AudioProcessorListener,
                                   private juce::AsyncUpdater
    {
    public:
        MorpherProcessor();
        ~MorpherProcessor() override;

        //==========================================================================
        // AudioProcessor
        void prepareToPlay (double sampleRate, int maxBlockSize) override;
        void releaseResources() override;
        void reset() override;
        bool isBusesLayoutSupported (const BusesLayout&) const override;
        using AudioProcessor::processBlock;
        using AudioProcessor::processBlockBypassed;
        void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
        void processBlockBypassed (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
        void setNonRealtime (bool) noexcept override;

        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override                     { return true; }

        const juce::String getName() const override         { return "Morpher"; }
        bool acceptsMidi() const override                   { return true; }
        bool producesMidi() const override                  { return false; }
        bool isMidiEffect() const override                  { return false; }
        double getTailLengthSeconds() const override;

        int getNumPrograms() override                       { return 1; }
        int getCurrentProgram() override                    { return 0; }
        void setCurrentProgram (int) override               {}
        const juce::String getProgramName (int) override    { return {}; }
        void changeProgramName (int, const juce::String&) override {}

        void getStateInformation (juce::MemoryBlock&) override;
        void setStateInformation (const void*, int) override;

        //==========================================================================
        // The pad. Both run 0 to 1; y = 1 is the top (decision 0006).
        juce::RangedAudioParameter& getXParameter() const   { return *xParam; }
        juce::RangedAudioParameter& getYParameter() const   { return *yParam; }

        /** Where the sound is now, after smoothing, for drawing. */
        float getCurrentX() const noexcept                  { return shownX.load(); }
        float getCurrentY() const noexcept                  { return shownY.load(); }

        //==========================================================================
        // The corners. Message thread only.

        enum class SlotStatus { empty, loaded, missing };

        SlotStatus getStatus (int slot) const;
        juce::String getPluginName (int slot) const;          // empty if none
        juce::PluginDescription getDescription (int slot) const;
        juce::AudioPluginInstance* getInstance (int slot) const;
        juce::String getProblem (int slot) const;             // why a plug-in is missing

        /** Loads a plug-in into a corner, with saved settings if given.
            Returns an empty string, or what went wrong in plain words. */
        juce::String load (int slot, const juce::PluginDescription&, const juce::MemoryBlock* state = nullptr);

        /** Every plug-in a file holds (usually one). */
        juce::OwnedArray<juce::PluginDescription> findPluginsIn (const juce::File&);

        void remove (int slot);
        SlotContents capture (int slot) const;
        void restore (int slot, const SlotContents&);

        /** Loads, and on success leaves an Undo that puts the old one back. */
        juce::String loadWithUndo (int slot, const juce::PluginDescription&);
        /** Removes, leaving an Undo. */
        void removeWithUndo (int slot);

        juce::AudioPluginFormatManager& getFormatManager()  { return formatManager; }

        //==========================================================================
        // The message line.
        const Notice& getNotice() const noexcept            { return notice; }
        void setNotice (Notice);
        void dismissNotice();
        /** Runs the notice's action (Undo), clearing the notice. */
        void runNoticeAction();

        /** Says when the corners or the notice change. Message thread. */
        juce::ChangeBroadcaster changes;

        /** Told just before a corner's plug-in goes, so its window can close first. */
        struct SlotListener
        {
            virtual ~SlotListener() = default;
            virtual void slotWillChange (int slot) = 0;
        };

        void addSlotListener (SlotListener* l)          { slotListeners.add (l); }
        void removeSlotListener (SlotListener* l)       { slotListeners.remove (l); }

        /** For the tests: the delays each corner is held back by. */
        int getSlotDelay (int slot) const                   { return slots[(size_t) slot].getDelay(); }

    private:
        struct SlotInfo
        {
            SlotStatus status = SlotStatus::empty;
            juce::PluginDescription description;
            juce::MemoryBlock keptState;       // a missing plug-in's settings, kept for when it's back
            juce::String problem;
        };

        juce::String loadUnlocked (int slot, const juce::PluginDescription&,
                                   const juce::MemoryBlock* state, bool keepIfMissing);
        void removeUnlocked (int slot);
        std::unique_ptr<juce::AudioPluginInstance> takeOut (int slot, std::unique_ptr<juce::AudioPluginInstance> next);
        SlotContents captureUnlocked (int slot) const;
        void applyState (const juce::XmlElement&);
        std::unique_ptr<juce::XmlElement> createStateXml();
        void updateLatency();
        void slotsChanged();
        void userChangedSlots();
        juce::PluginDescription locateMoved (const juce::PluginDescription&) const;

        // AudioProcessorListener, for the hosted plug-ins.
        void audioProcessorChanged (AudioProcessor*, const juce::AudioProcessorListener::ChangeDetails&) override;
        void audioProcessorParameterChanged (AudioProcessor*, int, float) override;
        void handleAsyncUpdate() override;

        juce::AudioProcessorValueTreeState parameters;
        juce::RangedAudioParameter* xParam = nullptr;
        juce::RangedAudioParameter* yParam = nullptr;

        juce::AudioPluginFormatManager formatManager;

        std::array<Slot, numSlots> slots;
        std::array<SlotInfo, numSlots> info;

        // Held while a corner's plug-in changes or its state is read, so a
        // host saving from another thread never sees one half-swapped. The
        // audio thread never takes it.
        juce::CriticalSection stateLock;
        std::unique_ptr<juce::XmlElement> pendingState;   // set off the message thread, applied on it

        // Audio thread.
        juce::SmoothedValue<float> smoothX, smoothY;
        juce::AudioBuffer<float> dry, mixed, slotOut;
        juce::HeapBlock<float> weights;
        juce::MidiBuffer chunkMidi;
        DelayLine bypassDelay;
        int preparedBlock = 0;
        std::atomic<float> shownX { 0.0f }, shownY { 1.0f };
        std::atomic<bool> latencyDirty { false }, stateDirty { false };
        std::atomic<double> tailSeconds { 0.0 };

        Notice notice;
        juce::ListenerList<SlotListener> slotListeners;

        std::atomic<float>* xValue = nullptr;
        std::atomic<float>* yValue = nullptr;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MorpherProcessor)
    };
}
