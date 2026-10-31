#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace morpher
{
    /** A hosted plug-in's own window. Morpher's window owns these, so they
        close with it; each one's title names the plug-in and its corner. */
    class PluginWindow final : public juce::DocumentWindow
    {
    public:
        PluginWindow (juce::AudioPluginInstance&, const juce::String& title, std::function<void()> onClose);
        ~PluginWindow() override;

        void closeButtonPressed() override;
        float getDesktopScaleFactor() const override   { return 1.0f; }

    private:
        std::function<void()> closed;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginWindow)
    };
}
