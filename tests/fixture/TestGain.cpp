// A test plug-in: out = in * gain, delayed by latency samples. Both are
// parameters, so a test can give each corner a different one, and a change of
// latency is reported to the host as a real plug-in's would be.

#include <juce_audio_processors/juce_audio_processors.h>

class TestGain final : public juce::AudioProcessor,
                       private juce::AudioProcessorParameter::Listener
{
public:
    TestGain()
        : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                           .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    {
        addParameter (gainParam = new juce::AudioParameterFloat (juce::ParameterID { "gain", 1 }, "Gain",
                                                                 0.0f, 1.0f, 1.0f));
        addParameter (latencyParam = new juce::AudioParameterInt (juce::ParameterID { "latency", 1 }, "Latency",
                                                                  0, 4000, 0));
        latencyParam->addListener (this);
    }

    ~TestGain() override { latencyParam->removeListener (this); }

    void parameterValueChanged (int, float) override
    {
        latency = latencyParam->get();
        setLatencySamples (latency);
    }

    void parameterGestureChanged (int, bool) override {}

    void prepareToPlay (double, int) override
    {
        line.setSize (2, 1 << 16);
        line.clear();
        pos = 0;
    }

    void releaseResources() override {}

    bool isBusesLayoutSupported (const BusesLayout& l) const override
    {
        return l.getMainInputChannelSet() == l.getMainOutputChannelSet() && l.getMainOutputChannelSet().size() <= 2;
    }

    using AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>& b, juce::MidiBuffer&) override
    {
        const auto g = gainParam->get();
        const auto d = latency.load();
        const auto size = line.getNumSamples();
        int end = pos;

        for (int ch = 0; ch < juce::jmin (2, b.getNumChannels()); ++ch)
        {
            auto* data = b.getWritePointer (ch);
            auto* store = line.getWritePointer (ch);
            int w = pos;

            for (int i = 0; i < b.getNumSamples(); ++i)
            {
                store[w] = data[i] * g;
                data[i] = store[(w - d + size) % size];
                w = (w + 1) % size;
            }

            end = w;
        }

        pos = end;
    }

    juce::AudioProcessorEditor* createEditor() override   { return new juce::GenericAudioProcessorEditor (*this); }
    bool hasEditor() const override                       { return true; }
    const juce::String getName() const override           { return "Morpher Test Gain"; }
    bool acceptsMidi() const override                     { return false; }
    bool producesMidi() const override                    { return false; }
    double getTailLengthSeconds() const override          { return 0.0; }
    int getNumPrograms() override                         { return 1; }
    int getCurrentProgram() override                      { return 0; }
    void setCurrentProgram (int) override                 {}
    const juce::String getProgramName (int) override      { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& dest) override
    {
        juce::XmlElement xml ("TESTGAIN");
        xml.setAttribute ("gain", (double) gainParam->get());
        xml.setAttribute ("latency", latency.load());
        copyXmlToBinary (xml, dest);
    }

    void setStateInformation (const void* data, int size) override
    {
        if (auto xml = getXmlFromBinary (data, size))
        {
            *gainParam = (float) xml->getDoubleAttribute ("gain", 1.0);
            *latencyParam = juce::jlimit (0, 4000, xml->getIntAttribute ("latency", 0));
        }
    }

private:
    juce::AudioParameterFloat* gainParam = nullptr;
    juce::AudioParameterInt* latencyParam = nullptr;
    std::atomic<int> latency { 0 };
    juce::AudioBuffer<float> line;
    int pos = 0;
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TestGain();
}
