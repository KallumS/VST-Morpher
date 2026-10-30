#include <juce_audio_processors/juce_audio_processors.h>
#include "MorpherProcessor.h"
#include "Theme.h"
#include "ui/MorpherEditor.h"

using namespace morpher;

/** The window, checked against the sizes and rules in Apple's guidelines
    (docs/apple-hig-review.md): every case at every text size. */
struct EditorTests final : juce::UnitTest
{
    EditorTests() : juce::UnitTest ("The window (Apple's guidelines)", "Morpher") {}

    static juce::Array<juce::Component*> allVisible (juce::Component& root)
    {
        juce::Array<juce::Component*> out;
        std::function<void (juce::Component&)> walk = [&] (juce::Component& c)
        {
            for (auto* child : c.getChildren())
                if (child->isVisible())
                {
                    out.add (child);
                    walk (*child);
                }
        };
        walk (root);
        return out;
    }

    /** A component's bounds in the content's points. */
    static juce::Rectangle<int> inContent (juce::Component& c, juce::Component& content)
    {
        return content.getLocalArea (&c, c.getLocalBounds());
    }

    void checkLayout (MorpherEditor& editor, const juce::String& where)
    {
        auto& content = editor.getContent();
        const auto bounds = content.getLocalBounds();
        juce::Array<juce::Rectangle<int>> bezels;

        for (auto* c : allVisible (content))
        {
            const auto r = inContent (*c, content);

            // Nothing runs off the edge (pad and bar overhang only by the ring's margin).
            expect (bounds.expanded (theme::ringMargin).contains (r), where + ": " + c->getTitle() + " runs off the window " + r.toString());

            if (auto* b = dynamic_cast<HouseButton*> (c))
            {
                const auto bezel = r.reduced (theme::ringMargin);
                expectEquals (bezel.getHeight(), theme::controlHeight, where + ": " + b->getButtonText() + " is 28 tall");
                expect (bezel.getWidth() >= theme::controlHeight, where + ": " + b->getButtonText() + " is at least square");

                const auto labelWidth = juce::GlyphArrangement::getStringWidthInt (theme::bodyFont(), b->getButtonText());
                expect (bezel.getWidth() >= labelWidth + 2 * theme::buttonPadding,
                        where + ": " + b->getButtonText() + " is wide enough for its label");

                expect (b->getWantsKeyboardFocus(), where + ": " + b->getButtonText() + " is reachable with Tab");
                expect (b->getTitle().isNotEmpty() || b->getButtonText().isNotEmpty(), where + ": a button with no name");
                expect (b->getTooltip().isNotEmpty(), where + ": " + b->getButtonText() + " has a tooltip");

                for (auto& other : bezels)
                {
                    expect (! other.intersects (bezel), where + ": buttons overlap");

                    // Measured edge to edge: halving an odd width to find a
                    // centre would come out a point short.
                    const auto sameRow = other.getY() == bezel.getY();
                    const auto sameColumn = other.getX() == bezel.getX();
                    if (sameRow)
                        expect (juce::jmax (other.getX() - bezel.getRight(), bezel.getX() - other.getRight()) >= theme::controlGap,
                                where + ": " + b->getButtonText() + " is 12 from its neighbour");
                    if (sameColumn)
                        expect (juce::jmax (other.getY() - bezel.getBottom(), bezel.getY() - other.getBottom()) >= theme::controlGap,
                                where + ": " + b->getButtonText() + " is 12 below the one above");
                }

                bezels.add (bezel);
            }
        }

        // The big pieces don't overlap one another.
        juce::Array<juce::Component*> pieces { &editor.getPad() };
        for (int s = 0; s < numSlots; ++s)
            pieces.add (&editor.getPanel (s));
        if (editor.getMessageBar().isVisible())
            pieces.add (&editor.getMessageBar());

        for (int i = 0; i < pieces.size(); ++i)
            for (int j = i + 1; j < pieces.size(); ++j)
            {
                auto a = inContent (*pieces[i], content), b = inContent (*pieces[j], content);
                if (pieces[i] == &editor.getPad() || pieces[i] == &editor.getMessageBar()) a = a.reduced (theme::ringMargin, 0);
                if (pieces[j] == &editor.getPad() || pieces[j] == &editor.getMessageBar()) b = b.reduced (theme::ringMargin, 0);
                expect (! a.intersects (b), where + ": " + pieces[i]->getTitle() + " overlaps " + pieces[j]->getTitle());
            }

        // Each panel is tall enough for its buttons.
        for (int s = 0; s < numSlots; ++s)
        {
            auto& panel = editor.getPanel (s);
            expect (panel.getHeight() >= panel.getIdealHeight (panel.getWidth()), where + ": a panel is cut short");
            expect (! panel.chooseButton.getBounds().isEmpty());
        }

        expect (editor.getPad().getHeight() >= 100, where + ": the pad is too small to use: " + juce::String (editor.getPad().getHeight()));

        // The message is above everything else.
        if (editor.getMessageBar().isVisible())
        {
            const auto barY = editor.getMessageBar().getY();
            const auto padY = editor.getPad().getY();
            expect (barY < padY, where + ": the message is under the pad");
            for (int s = 0; s < numSlots; ++s)
                expect (barY < editor.getPanel (s).getY(), where + ": the message is under a panel");
        }

        // A panel sits beside or by its own corner.
        const auto padCentre = editor.getPad().getBounds().getCentre();
        expect (editor.getPanel (cornerA).getBounds().getCentreX() < padCentre.x, where + ": A is left of the pad's middle");
        expect (editor.getPanel (cornerB).getBounds().getCentreX() > padCentre.x, where + ": B is right");
        expect (editor.getPanel (cornerC).getBounds().getCentreY() > editor.getPanel (cornerA).getBounds().getCentreY(), where + ": C is below A");
        expect (editor.getPanel (cornerD).getBounds().getCentreY() > editor.getPanel (cornerB).getBounds().getCentreY(), where + ": D is below B");
    }

    void runTest() override
    {
        MorpherProcessor p;
        p.setRateAndBufferSizeDetails (48000.0, 256);
        p.prepareToPlay (48000.0, 256);

        auto found = p.findPluginsIn (juce::File (MORPHER_TEST_PLUGIN));
        const auto d = *found[0];

        std::unique_ptr<MorpherEditor> editor (dynamic_cast<MorpherEditor*> (p.createEditor()));
        expect (editor != nullptr);

        const juce::String longMessage = "Couldn't open Pro-Q 3 (A), Saturn 2 (B). Their settings are kept and come back "
                                         "when they're installed, or choose another plug-in in its place.";

        struct Case { const char* name; int loaded; bool missing; bool message; };
        const Case cases[] { { "empty", 0, false, false },
                             { "two loaded", 2, false, false },
                             { "all loaded, a message", 4, false, true },
                             { "one missing, a message", 3, true, true } };

        beginTest ("Every case at every text size and window size: 28-point buttons 12 apart, "
                   "wide enough, nothing overlapping or off the edge, the message on top");

        for (const auto& c : cases)
        {
            for (int s = 0; s < numSlots; ++s)
                p.remove (s);

            for (int s = 0; s < c.loaded; ++s)
                p.load (s, d);

            if (c.missing)
            {
                juce::MemoryBlock saved;
                p.getStateInformation (saved);
                auto xml = juce::AudioProcessor::getXmlFromBinary (saved.getData(), (int) saved.getSize());
                auto* slot = xml->createNewChildElement ("SLOT");
                slot->setAttribute ("index", 3);
                auto gone = d;
                gone.fileOrIdentifier = "/nowhere/Gone.vst3";
                gone.name = "A Plug-in With Quite A Long Name Indeed";
                slot->addChildElement (gone.createXml().release());
                juce::AudioProcessor::copyXmlToBinary (*xml, saved);
                p.setStateInformation (saved.getData(), (int) saved.getSize());
                expect (p.getStatus (3) == MorpherProcessor::SlotStatus::missing);
            }

            if (c.message)
                p.setNotice ({ longMessage, true, "Undo", [] {} });
            else
                p.dismissNotice();

            // The change message is asynchronous: deliver it.
            p.changes.dispatchPendingMessages();

            for (auto size : theme::textSizes)
            {
                editor->setTextSize (size);
                const auto s = (float) size / 100.0f;

                for (auto [w, h] : { std::pair { MorpherEditor::minWidth, MorpherEditor::minHeight },
                                     std::pair { MorpherEditor::wideFrom, MorpherEditor::minHeight },
                                     std::pair { MorpherEditor::defaultWidth, MorpherEditor::defaultHeight },
                                     std::pair { 1400, 900 } })
                {
                    editor->setSize (juce::roundToInt ((float) w * s), juce::roundToInt ((float) h * s));
                    const auto where = juce::String (c.name) + " at " + juce::String (size) + "%, "
                                       + juce::String (w) + "x" + juce::String (h);
                    expectEquals (editor->getContent().getWidth(), w, where);
                    checkLayout (*editor, where);
                }
            }
        }

        beginTest ("Text Size: 100% to 200%, remembered, the window growing with it");
        {
            juce::SharedResourcePointer<Settings> settings;
            editor->setTextSize (100);
            editor->setSize (MorpherEditor::defaultWidth, MorpherEditor::defaultHeight);
            editor->setTextSize (200);
            expectEquals (editor->getWidth(), MorpherEditor::defaultWidth * 2);
            expectEquals (editor->getContent().getWidth(), MorpherEditor::defaultWidth);
            expectEquals (settings->getTextSize(), 200);
            editor->setTextSize (170);
            expectEquals (editor->getTextSize(), 150, "an in-between size snaps to one offered");
            editor->setTextSize (100);
            expectEquals (editor->getWidth(), MorpherEditor::defaultWidth);

            expect (editor->keyPressed (juce::KeyPress ('=', juce::ModifierKeys::commandModifier, '=')));
            expectEquals (editor->getTextSize(), 125);
            expect (editor->keyPressed (juce::KeyPress ('-', juce::ModifierKeys::commandModifier, '-')));
            expectEquals (editor->getTextSize(), 100);
            editor->setTextSize (200);
            expect (editor->keyPressed (juce::KeyPress ('0', juce::ModifierKeys::commandModifier, '0')));
            expectEquals (editor->getTextSize(), 100);
        }

        beginTest ("The pad from the keyboard: arrows move it, Shift for small steps");
        {
            auto& pad = editor->getPad();
            p.getXParameter().setValueNotifyingHost (0.5f);
            p.getYParameter().setValueNotifyingHost (0.5f);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (50);
            expect (pad.getWantsKeyboardFocus());

            expect (pad.keyPressed (juce::KeyPress (juce::KeyPress::rightKey)));
            expectWithinAbsoluteError (p.getXParameter().getValue(), 0.55f, 1.0e-4f);
            expect (pad.keyPressed (juce::KeyPress (juce::KeyPress::upKey, juce::ModifierKeys::shiftModifier, 0)));
            expectWithinAbsoluteError (p.getYParameter().getValue(), 0.51f, 1.0e-4f);
            expect (pad.keyPressed (juce::KeyPress (juce::KeyPress::escapeKey)), "Esc lets go of the pad");
        }

        beginTest ("The pad follows the pointer, and its corners are where the sound is");
        {
            auto& pad = editor->getPad();
            const auto topLeft = pad.toScreen (0.0f, 1.0f), bottomRight = pad.toScreen (1.0f, 0.0f);
            expect (topLeft.x < bottomRight.x && topLeft.y < bottomRight.y);
            const auto v = pad.toValue (pad.toScreen (0.3f, 0.8f));
            expectWithinAbsoluteError (v.x, 0.3f, 1.0e-4f);
            expectWithinAbsoluteError (v.y, 0.8f, 1.0e-4f);
            expect (pad.getSquare().contains (topLeft) && pad.getSquare().contains (bottomRight), "the handle stays inside the pad");
        }

        beginTest ("Screen readers: titles, and the pad as two sliders");
        {
            // Screen readers only see a window that's on screen.
            editor->addToDesktop (0);
            editor->setVisible (true);
            auto& pad = editor->getPad();
            int sliders = 0;

            for (auto* child : pad.getChildren())
                if (auto* h = child->getAccessibilityHandler())
                    if (h->getRole() == juce::AccessibilityRole::slider && h->getValueInterface() != nullptr)
                    {
                        ++sliders;
                        expect (h->getTitle().startsWith ("Morph "));
                    }

            expectEquals (sliders, 2);
            expect (editor->getTitle() == "Morpher");

            for (int s = 0; s < numSlots; ++s)
                expect (editor->getPanel (s).getTitle().startsWith ("Corner " + juce::String (cornerName (s))));

            editor->removeFromDesktop();
        }

        beginTest ("A corner's level shows as it moves");
        {
            p.getXParameter().setValueNotifyingHost (0.0f);
            p.getYParameter().setValueNotifyingHost (1.0f);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (50);
            expectWithinAbsoluteError (editor->getPanel (cornerA).getState().level, 1.0f, 1.0e-4f);
            expectWithinAbsoluteError (editor->getPanel (cornerD).getState().level, 0.0f, 1.0e-4f);
        }

        beginTest ("The choose menu offers recent and installed plug-ins and a file");
        {
            juce::SharedResourcePointer<Settings> settings;
            settings->clearRecentPlugins();
            settings->addRecentPlugin (d);
            auto menu = editor->createChooseMenu (cornerA);
            juce::StringArray names;

            for (juce::PopupMenu::MenuItemIterator it (menu); it.next();)
                names.add (it.getItem().text);

            expect (names.contains ("Recent"), names.joinIntoString (" | "));
            expect (names.joinIntoString ("|").contains ("Morpher Test Gain"));
            expect (names.joinIntoString ("|").contains ("Choose File"));
            settings->clearRecentPlugins();
        }

        beginTest ("Edit opens the plug-in's window; removing the plug-in closes it first");
        {
            for (int s = 0; s < numSlots; ++s)
                p.remove (s);

            p.load (cornerB, d);
            p.changes.dispatchPendingMessages();

            editor->toggleWindow (cornerB);
            expect (editor->isWindowOpen (cornerB));
            expect (editor->getPanel (cornerB).editButton.getToggleState(), "Edit shows as on while the window is open");

            p.removeWithUndo (cornerB);
            expect (! editor->isWindowOpen (cornerB));
            p.runNoticeAction();
            p.changes.dispatchPendingMessages();
            expect (! editor->getPanel (cornerB).editButton.getToggleState());

            editor->toggleWindow (cornerB);
            expect (editor->isWindowOpen (cornerB));
            editor->toggleWindow (cornerB);
            expect (! editor->isWindowOpen (cornerB));
        }

        editor.reset();
        p.releaseResources();
    }
};

static EditorTests editorTests;
