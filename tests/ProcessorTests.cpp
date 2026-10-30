#include <juce_audio_processors/juce_audio_processors.h>
#include "MorpherProcessor.h"

using namespace morpher;

/** Loads the test plug-in (out = in * gain, delayed by latency) into
    Morpher's corners and checks what comes out. */
struct ProcessorTests final : juce::UnitTest
{
    ProcessorTests() : juce::UnitTest ("Hosting and morphing real VST3s", "Morpher") {}

    static constexpr double rate = 48000.0;
    static constexpr int block = 256;

    /** Pumps the message loop, as a host's would, for latency changes and the like. */
    static void pump()
    {
        juce::MessageManager::getInstance()->runDispatchLoopUntil (30);
    }

    /** Loads the test plug-in into a corner and sets its gain and latency, as
        you would in its own window. */
    juce::String loadTest (MorpherProcessor& p, int slot, float gain, int latency)
    {
        const auto error = p.load (slot, testPlugin (p));

        if (auto* inst = p.getInstance (slot))
        {
            for (auto* param : inst->getParameters())
            {
                if (param->getName (32) == "Gain")     param->setValueNotifyingHost (gain);
                if (param->getName (32) == "Latency")  param->setValueNotifyingHost ((float) latency / 4000.0f);
            }
        }

        pump();
        return error;
    }

    juce::PluginDescription testPlugin (MorpherProcessor& p)
    {
        auto found = p.findPluginsIn (juce::File (MORPHER_TEST_PLUGIN));
        expectEquals (found.size(), 1, "the test plug-in should hold one plug-in");
        return found.isEmpty() ? juce::PluginDescription() : *found[0];
    }

    static void moveTo (MorpherProcessor& p, float x, float y)
    {
        p.getXParameter().setValueNotifyingHost (x);
        p.getYParameter().setValueNotifyingHost (y);
    }

    /** Feeds a constant and returns the last output sample, once smoothing and delays have settled. */
    static float settle (MorpherProcessor& p, float input = 1.0f, int channel = 0)
    {
        juce::AudioBuffer<float> buffer (p.getMainBusNumOutputChannels(), block);
        juce::MidiBuffer midi;

        for (int i = 0; i < 20; ++i)
        {
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                juce::FloatVectorOperations::fill (buffer.getWritePointer (ch), input, block);

            p.processBlock (buffer, midi);
        }

        return buffer.getSample (channel, block - 1);
    }

    /** Sends one impulse and returns where every non-zero output sample lands. */
    static std::vector<std::pair<int, float>> impulse (MorpherProcessor& p, int length)
    {
        p.reset();
        juce::AudioBuffer<float> buffer (2, block);
        juce::MidiBuffer midi;
        std::vector<std::pair<int, float>> hits;

        for (int start = 0; start < length; start += block)
        {
            buffer.clear();

            if (start == 0)
                buffer.setSample (0, 0, 1.0f);

            p.processBlock (buffer, midi);

            for (int i = 0; i < block; ++i)
                if (std::abs (buffer.getSample (0, i)) > 1.0e-6f)
                    hits.emplace_back (start + i, buffer.getSample (0, i));
        }

        return hits;
    }

    void prepare (MorpherProcessor& p, int numChannels = 2)
    {
        const auto set = juce::AudioChannelSet::canonicalChannelSet (numChannels);
        juce::AudioProcessor::BusesLayout layout;
        layout.inputBuses.add (set);
        layout.outputBuses.add (set);
        expect (p.setBusesLayout (layout));
        p.setRateAndBufferSizeDetails (rate, block);
        p.prepareToPlay (rate, block);
    }

    void runTest() override
    {
        beginTest ("The test plug-in was built");
        expect (juce::File (MORPHER_TEST_PLUGIN).exists(), MORPHER_TEST_PLUGIN);

        beginTest ("New: every corner empty, it passes the sound through, starting at A");
        {
            MorpherProcessor p;
            prepare (p);
            expectEquals (p.getXParameter().getValue(), 0.0f);
            expectEquals (p.getYParameter().getValue(), 1.0f);
            expectWithinAbsoluteError (settle (p, 0.7f), 0.7f, 1.0e-6f);
            expectEquals (p.getLatencySamples(), 0);
        }

        beginTest ("Each corner is heard alone at its corner, and mixed between");
        {
            MorpherProcessor p;
            prepare (p);
            expect (loadTest (p, cornerA, 0.5f, 0).isEmpty());
            expect (loadTest (p, cornerB, 0.25f, 0).isEmpty());
            expect (p.getStatus (cornerA) == MorpherProcessor::SlotStatus::loaded);
            expectEquals (p.getPluginName (cornerA), juce::String ("Morpher Test Gain"));

            moveTo (p, 0.0f, 1.0f);   expectWithinAbsoluteError (settle (p), 0.5f,  1.0e-5f);
            moveTo (p, 1.0f, 1.0f);   expectWithinAbsoluteError (settle (p), 0.25f, 1.0e-5f);
            moveTo (p, 0.0f, 0.0f);   expectWithinAbsoluteError (settle (p), 1.0f,  1.0e-5f);   // C is empty: dry
            moveTo (p, 0.5f, 1.0f);   expectWithinAbsoluteError (settle (p), 0.375f, 1.0e-5f);
            moveTo (p, 0.5f, 0.5f);   expectWithinAbsoluteError (settle (p), 0.25f * (0.5f + 0.25f + 1.0f + 1.0f), 1.0e-5f);
            expectWithinAbsoluteError (settle (p, 1.0f, 1), 0.6875f, 1.0e-5f, "the right channel too");
        }

        beginTest ("Corners are lined up in time with the slowest, and the host is told");
        {
            MorpherProcessor p;
            prepare (p);
            loadTest (p, cornerA, 0.5f, 0);
            loadTest (p, cornerB, 0.25f, 100);

            expectEquals (p.getLatencySamples(), 100);
            expectEquals (p.getSlotDelay (cornerA), 100);
            expectEquals (p.getSlotDelay (cornerB), 0);
            expectEquals (p.getSlotDelay (cornerC), 100);

            moveTo (p, 0.5f, 0.5f);
            settle (p, 0.0f);
            const auto hits = impulse (p, 2048);
            expectEquals ((int) hits.size(), 1, "one click in, one click out: no echoes");

            if (! hits.empty())
            {
                expectEquals (hits[0].first, 100);
                expectWithinAbsoluteError (hits[0].second, 0.6875f, 1.0e-5f);
            }

            p.remove (cornerB);
            expectEquals (p.getLatencySamples(), 0, "the delay goes when the slow plug-in does");
        }

        beginTest ("Bypassed, the dry sound keeps the same delay");
        {
            MorpherProcessor p;
            prepare (p);
            loadTest (p, cornerB, 1.0f, 64);
            juce::AudioBuffer<float> buffer (2, block);
            juce::MidiBuffer midi;
            buffer.clear();
            buffer.setSample (0, 0, 1.0f);
            p.processBlockBypassed (buffer, midi);
            expectEquals (buffer.getSample (0, 0), 0.0f);
            expectEquals (buffer.getSample (0, 64), 1.0f);
        }

        beginTest ("A host block longer than promised is taken in pieces");
        {
            MorpherProcessor p;
            prepare (p);
            loadTest (p, cornerA, 0.5f, 0);
            settle (p);
            juce::AudioBuffer<float> big (2, block * 4 + 17);
            juce::MidiBuffer midi;

            for (int ch = 0; ch < 2; ++ch)
                juce::FloatVectorOperations::fill (big.getWritePointer (ch), 1.0f, big.getNumSamples());

            p.processBlock (big, midi);
            expectWithinAbsoluteError (big.getSample (0, 0), 0.5f, 1.0e-5f);
            expectWithinAbsoluteError (big.getSample (1, big.getNumSamples() - 1), 0.5f, 1.0e-5f);
        }

        beginTest ("Mono works too");
        {
            MorpherProcessor p;
            prepare (p, 1);
            loadTest (p, cornerA, 0.5f, 0);
            expectWithinAbsoluteError (settle (p), 0.5f, 1.0e-5f);
        }

        beginTest ("Only mono and stereo, the same in as out");
        {
            MorpherProcessor p;
            juce::AudioProcessor::BusesLayout l;
            l.inputBuses.add (juce::AudioChannelSet::stereo());
            l.outputBuses.add (juce::AudioChannelSet::mono());
            expect (! p.checkBusesLayoutSupported (l));
            l.inputBuses.getReference (0) = juce::AudioChannelSet::create5point1();
            l.outputBuses.getReference (0) = juce::AudioChannelSet::create5point1();
            expect (! p.checkBusesLayoutSupported (l));
        }

        beginTest ("Saving and reopening gives back the plug-ins, their settings and the pad");
        {
            juce::MemoryBlock saved;

            {
                MorpherProcessor p;
                prepare (p);
                loadTest (p, cornerA, 0.5f, 0);
                loadTest (p, cornerD, 0.25f, 32);
                moveTo (p, 0.3f, 0.6f);
                p.getStateInformation (saved);
            }

            MorpherProcessor q;
            prepare (q);
            q.setStateInformation (saved.getData(), (int) saved.getSize());
            pump();

            expect (q.getStatus (cornerA) == MorpherProcessor::SlotStatus::loaded);
            expect (q.getStatus (cornerB) == MorpherProcessor::SlotStatus::empty);
            expect (q.getStatus (cornerD) == MorpherProcessor::SlotStatus::loaded);
            expectWithinAbsoluteError (q.getXParameter().getValue(), 0.3f, 1.0e-4f);
            expectWithinAbsoluteError (q.getYParameter().getValue(), 0.6f, 1.0e-4f);
            expectEquals (q.getLatencySamples(), 32);

            const auto w = weightsAt (0.3f, 0.6f);
            const auto expected = w[cornerA] * 0.5f + w[cornerB] + w[cornerC] + w[cornerD] * 0.25f;
            expectWithinAbsoluteError (settle (q), expected, 1.0e-4f);
        }

        beginTest ("A plug-in that isn't installed is kept, said so, and saved again unchanged");
        {
            MorpherProcessor p;
            prepare (p);
            loadTest (p, cornerA, 0.5f, 0);

            juce::MemoryBlock saved;
            p.getStateInformation (saved);
            auto xml = juce::AudioProcessor::getXmlFromBinary (saved.getData(), (int) saved.getSize());
            auto* plugin = xml->getChildByName ("SLOT")->getChildByName ("PLUGIN");
            plugin->setAttribute ("file", "/nowhere/Gone.vst3");
            plugin->setAttribute ("name", "Gone");
            juce::MemoryBlock broken;
            juce::AudioProcessor::copyXmlToBinary (*xml, broken);

            MorpherProcessor q;
            prepare (q);
            q.setStateInformation (broken.getData(), (int) broken.getSize());
            expect (q.getStatus (cornerA) == MorpherProcessor::SlotStatus::missing);
            expect (q.getNotice().isWarning);
            expect (q.getNotice().text.contains ("Gone"), q.getNotice().text);
            expect (q.getNotice().text.contains ("kept"), q.getNotice().text);
            expectWithinAbsoluteError (settle (q, 0.8f), 0.8f, 1.0e-6f, "a missing corner passes the sound through");

            juce::MemoryBlock again;
            q.getStateInformation (again);
            auto xml2 = juce::AudioProcessor::getXmlFromBinary (again.getData(), (int) again.getSize());
            auto* slot2 = xml2->getChildByName ("SLOT");
            expect (slot2 != nullptr && slot2->getChildByName ("PLUGIN")->getStringAttribute ("name") == "Gone");
            expectEquals (slot2->getChildElementAllSubText ("STATE", {}),
                          xml->getChildByName ("SLOT")->getChildElementAllSubText ("STATE", {}),
                          "its settings are saved unchanged");
        }

        beginTest ("Remove can be undone, settings and all");
        {
            MorpherProcessor p;
            prepare (p);
            loadTest (p, cornerA, 0.5f, 0);

            p.removeWithUndo (cornerA);
            expect (p.getStatus (cornerA) == MorpherProcessor::SlotStatus::empty);
            expectEquals (p.getNotice().actionName, juce::String ("Undo"));
            expect (! p.getNotice().isWarning);
            expectWithinAbsoluteError (settle (p), 1.0f, 1.0e-6f);

            p.runNoticeAction();
            expect (p.getStatus (cornerA) == MorpherProcessor::SlotStatus::loaded);
            expectWithinAbsoluteError (settle (p), 0.5f, 1.0e-5f);
            expect (p.getNotice().text.isEmpty(), "Undo clears its message");
        }

        beginTest ("Replacing can be undone; loading into an empty corner says nothing");
        {
            MorpherProcessor p;
            prepare (p);
            const auto d = testPlugin (p);

            loadTest (p, cornerA, 0.5f, 0);
            expect (p.loadWithUndo (cornerB, d).isEmpty());
            expect (p.getNotice().text.isEmpty());

            expect (p.loadWithUndo (cornerA, d).isEmpty());     // a fresh one, gain 1
            expectEquals (p.getNotice().actionName, juce::String ("Undo"));
            expect (p.getNotice().text.startsWith ("Replaced"), p.getNotice().text);
            expectWithinAbsoluteError (settle (p), 1.0f, 1.0e-5f);

            p.runNoticeAction();
            expectWithinAbsoluteError (settle (p), 0.5f, 1.0e-5f);
        }

        beginTest ("A file that isn't a plug-in is refused in plain words");
        {
            MorpherProcessor p;
            const auto notAPlugin = juce::File::createTempFile (".vst3");
            notAPlugin.replaceWithText ("hello");
            expect (p.findPluginsIn (notAPlugin).isEmpty());
            notAPlugin.deleteFile();

            juce::PluginDescription bogus;
            bogus.name = "Bogus";
            bogus.pluginFormatName = "VST3";
            bogus.fileOrIdentifier = "/nowhere/Bogus.vst3";
            const auto error = p.loadWithUndo (cornerA, bogus);
            expect (error.startsWith ("Couldn't open Bogus."), error);
            expect (p.getStatus (cornerA) == MorpherProcessor::SlotStatus::empty, "a failed choice leaves the corner as it was");
            expect (p.getNotice().isWarning);
        }

        beginTest ("The host can automate the pad as Morph X and Morph Y");
        {
            MorpherProcessor p;
            expectEquals (p.getXParameter().getName (64), juce::String ("Morph X"));
            expectEquals (p.getYParameter().getName (64), juce::String ("Morph Y"));
            expectEquals (p.getXParameter().getText (0.25f, 16), juce::String ("25%"));
            expect (p.getXParameter().isAutomatable());
        }
    }
};

static ProcessorTests processorTests;
