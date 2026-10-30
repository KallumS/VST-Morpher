#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace morpher
{
    /** What Morpher remembers between sessions, shared by every instance:
        the text size, the plug-ins you have chosen before, and the folder
        you last chose one from. Kept in a file in the user's own settings
        folder; nothing is sent anywhere.

        Use it through juce::SharedResourcePointer<Settings>.
    */
    class Settings
    {
    public:
        Settings();
        ~Settings();

        int  getTextSize() const;               // a percentage from theme::textSizes
        void setTextSize (int percent);
        static int clampTextSize (int percent);

        /** Most recent first. */
        juce::Array<juce::PluginDescription> getRecentPlugins() const;
        void addRecentPlugin (const juce::PluginDescription&);
        void clearRecentPlugins();
        static constexpr int maxRecentPlugins = 12;

        juce::File getLastFolder() const;
        void setLastFolder (const juce::File&);

        /** Lets the tests keep their settings away from the user's. */
        static void useFileForTesting (const juce::File&);

    private:
        juce::PropertiesFile& file() const;

        std::unique_ptr<juce::PropertiesFile> properties;
        static juce::File testFile;

        JUCE_DECLARE_NON_COPYABLE (Settings)
    };
}
