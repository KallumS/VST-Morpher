#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace morpher
{
    /** The house scheme on JUCE's controls: light grey buttons lettered in
        ink, the accent only for what is on (with an ink bar under its label,
        so it isn't shown by colour alone), menus and tooltips lighter than
        the window, and a near-white ring where the keyboard is.
    */
    class HouseLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        HouseLookAndFeel();

        juce::Font getTextButtonFont (juce::TextButton&, int) override;
        int getTextButtonWidthToFitText (juce::TextButton&, int buttonHeight) override;
        void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                                   bool highlighted, bool down) override;
        void drawButtonText (juce::Graphics&, juce::TextButton&, bool highlighted, bool down) override;

        juce::Font getPopupMenuFont() override;
        void drawPopupMenuBackgroundWithOptions (juce::Graphics&, int w, int h, const juce::PopupMenu::Options&) override;

        juce::Font getMenuBarFont (juce::MenuBarComponent&, int itemIndex, const juce::String& itemText) override;
        void drawMenuBarBackground (juce::Graphics&, int w, int h, bool isMouseOverBar, juce::MenuBarComponent&) override;
        void drawMenuBarItem (juce::Graphics&, int w, int h, int itemIndex, const juce::String& itemText,
                              bool isMouseOverItem, bool isMenuOpen, bool isMouseOverBar, juce::MenuBarComponent&) override;

        juce::Rectangle<int> getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos,
                                               juce::Rectangle<int> parentArea) override;
        void drawTooltip (juce::Graphics&, const juce::String& text, int width, int height) override;

        void drawScrollbar (juce::Graphics&, juce::ScrollBar&, int x, int y, int w, int h, bool vertical,
                            int thumbStart, int thumbSize, bool mouseOver, bool mouseDown) override;

        void drawCornerResizer (juce::Graphics&, int w, int h, bool mouseOver, bool dragging) override;

        /** Breaks a tooltip into lines of about 64 characters. */
        static juce::String wrapTooltip (const juce::String&);

        /** Draws the keyboard's ring around an area. */
        static void drawFocusRing (juce::Graphics&, juce::Rectangle<float> area, float radius);
    };

    /** A button that also answers Space, as a Mac button does, and that
        doesn't take the keyboard ring when clicked. */
    class HouseButton final : public juce::TextButton
    {
    public:
        explicit HouseButton (const juce::String& label = {}, const juce::String& tip = {});

        void setTip (const juce::String&);
        bool keyPressed (const juce::KeyPress&) override;

        /** The width that fits the label, at least a square. */
        int getIdealWidth();
    };
}
