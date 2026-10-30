#include "Settings.h"
#include "Theme.h"

namespace morpher
{
    juce::File Settings::testFile;

    namespace keys
    {
        static constexpr const char* textSize = "textSize";
        static constexpr const char* recent   = "recentPlugins";
        static constexpr const char* folder   = "lastFolder";
    }

    Settings::Settings()
    {
        if (testFile != juce::File())
        {
            properties = std::make_unique<juce::PropertiesFile> (testFile, juce::PropertiesFile::Options());
            return;
        }

        juce::PropertiesFile::Options options;
        options.applicationName     = "Morpher";
        options.filenameSuffix      = ".settings";
        options.folderName          = "Morpher";
        options.osxLibrarySubFolder = "Application Support";
        options.storageFormat       = juce::PropertiesFile::storeAsXML;
        options.commonToAllUsers    = false;
        options.millisecondsBeforeSaving = 500;

        // Several instances, perhaps in several hosts, share one file.
        options.processLock = nullptr;

        properties = std::make_unique<juce::PropertiesFile> (options);
    }

    Settings::~Settings()
    {
        properties->saveIfNeeded();
    }

    void Settings::useFileForTesting (const juce::File& f) { testFile = f; }

    juce::PropertiesFile& Settings::file() const { return *properties; }

    int Settings::clampTextSize (int percent)
    {
        int best = theme::textSizes[0];

        for (auto size : theme::textSizes)
            if (std::abs (size - percent) < std::abs (best - percent))
                best = size;

        return best;
    }

    int Settings::getTextSize() const
    {
        file().reload();
        return clampTextSize (file().getIntValue (keys::textSize, 100));
    }

    void Settings::setTextSize (int percent)
    {
        file().setValue (keys::textSize, clampTextSize (percent));
        file().saveIfNeeded();
    }

    juce::Array<juce::PluginDescription> Settings::getRecentPlugins() const
    {
        juce::Array<juce::PluginDescription> result;
        file().reload();

        if (auto xml = file().getXmlValue (keys::recent))
        {
            for (auto* child : xml->getChildIterator())
            {
                juce::PluginDescription d;

                if (d.loadFromXml (*child))
                    result.add (d);
            }
        }

        return result;
    }

    void Settings::addRecentPlugin (const juce::PluginDescription& added)
    {
        auto list = getRecentPlugins();

        for (int i = list.size(); --i >= 0;)
            if (list.getReference (i).isDuplicateOf (added))
                list.remove (i);

        list.insert (0, added);

        juce::XmlElement xml ("RECENT");

        for (int i = 0; i < juce::jmin (list.size(), maxRecentPlugins); ++i)
            xml.addChildElement (list.getReference (i).createXml().release());

        file().setValue (keys::recent, &xml);
        file().saveIfNeeded();
    }

    void Settings::clearRecentPlugins()
    {
        file().removeValue (keys::recent);
        file().saveIfNeeded();
    }

    juce::File Settings::getLastFolder() const
    {
        const auto path = file().getValue (keys::folder);
        return juce::File::isAbsolutePath (path) ? juce::File (path) : juce::File();
    }

    void Settings::setLastFolder (const juce::File& folder)
    {
        file().setValue (keys::folder, folder.getFullPathName());
        file().saveIfNeeded();
    }
}
