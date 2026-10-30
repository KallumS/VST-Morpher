#include <juce_gui_basics/juce_gui_basics.h>
#include "Settings.h"

void writeScreenshots (const juce::File& folder);

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;

    if (argc == 3 && juce::String (argv[1]) == "--screenshots")
    {
        writeScreenshots (juce::File::getCurrentWorkingDirectory().getChildFile (argv[2]));
        return 0;
    }

    // Keep the tests' settings away from the user's own.
    const auto settingsFile = juce::File::createTempFile (".settings");
    morpher::Settings::useFileForTesting (settingsFile);

    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);
    runner.runAllTests();

    int failures = 0;

    for (int i = 0; i < runner.getNumResults(); ++i)
        failures += runner.getResult (i)->failures;

    settingsFile.deleteFile();
    std::printf ("\n%s: %d failure(s)\n", failures == 0 ? "PASSED" : "FAILED", failures);
    return failures == 0 ? 0 : 1;
}
