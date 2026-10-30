// Draws the window in a few states to PNGs, to look at (and for the docs).
// Run: MorpherTests --screenshots <folder>

#include <juce_audio_processors/juce_audio_processors.h>
#include "MorpherProcessor.h"
#include "ui/MorpherEditor.h"

using namespace morpher;

void writeScreenshots (const juce::File& folder)
{
    folder.createDirectory();

    MorpherProcessor p;
    p.setRateAndBufferSizeDetails (48000.0, 256);
    p.prepareToPlay (48000.0, 256);

    auto found = p.findPluginsIn (juce::File (MORPHER_TEST_PLUGIN));
    if (found.isEmpty())
        return;

    auto save = [&] (MorpherEditor& e, const juce::String& name)
    {
        p.changes.dispatchPendingMessages();
        juce::MessageManager::getInstance()->runDispatchLoopUntil (20);
        const auto image = e.createComponentSnapshot (e.getLocalBounds(), true, 1.0f);
        juce::FileOutputStream out (folder.getChildFile (name + ".png"));
        out.setPosition (0);
        out.truncate();
        juce::PNGImageFormat().writeImageToStream (image, out);
    };

    std::unique_ptr<MorpherEditor> editor (dynamic_cast<MorpherEditor*> (p.createEditor()));
    editor->setTextSize (100);

    save (*editor, "empty");

    p.load (cornerA, *found[0]);
    p.load (cornerB, *found[0]);
    p.load (cornerD, *found[0]);
    p.getXParameter().setValueNotifyingHost (0.35f);
    p.getYParameter().setValueNotifyingHost (0.7f);
    p.removeWithUndo (cornerD);
    save (*editor, "default");

    editor->setSize (MorpherEditor::minWidth, MorpherEditor::minHeight);
    save (*editor, "smallest");

    editor->setSize (1100, 600);
    save (*editor, "wide");

    editor->setSize (MorpherEditor::defaultWidth, MorpherEditor::defaultHeight);
    editor->setTextSize (200);
    save (*editor, "default-200");
    editor->setTextSize (100);
    editor.reset();
}
