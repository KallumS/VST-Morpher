#include "HouseLookAndFeel.h"
#include "../Theme.h"

namespace morpher
{
    using namespace juce;

    HouseLookAndFeel::HouseLookAndFeel()
    {
        auto scheme = getDarkColourScheme();
        scheme.setUIColour (ColourScheme::windowBackground,     theme::ground);
        scheme.setUIColour (ColourScheme::widgetBackground,     theme::sunken);
        scheme.setUIColour (ColourScheme::menuBackground,       theme::raised);
        scheme.setUIColour (ColourScheme::outline,              theme::rule);
        scheme.setUIColour (ColourScheme::defaultText,          theme::text);
        scheme.setUIColour (ColourScheme::defaultFill,          theme::control);
        scheme.setUIColour (ColourScheme::highlightedText,      theme::text);
        scheme.setUIColour (ColourScheme::highlightedFill,      theme::rule);
        scheme.setUIColour (ColourScheme::menuText,             theme::text);
        setColourScheme (scheme);

        setColour (ResizableWindow::backgroundColourId,         theme::ground);
        setColour (DocumentWindow::textColourId,                theme::text);

        setColour (TextButton::buttonColourId,                  theme::control);
        setColour (TextButton::buttonOnColourId,                theme::accent);
        setColour (TextButton::textColourOffId,                 theme::ink);
        setColour (TextButton::textColourOnId,                  theme::ink);

        setColour (Label::textColourId,                         theme::text);

        setColour (PopupMenu::backgroundColourId,               theme::raised);
        setColour (PopupMenu::textColourId,                     theme::text);
        setColour (PopupMenu::headerTextColourId,               theme::textOnRaised);
        setColour (PopupMenu::highlightedBackgroundColourId,    theme::rule);
        setColour (PopupMenu::highlightedTextColourId,          theme::text);

        setColour (TooltipWindow::backgroundColourId,           theme::raised);
        setColour (TooltipWindow::textColourId,                 theme::text);
        setColour (TooltipWindow::outlineColourId,              theme::rule);

        setColour (ScrollBar::thumbColourId,                    theme::scrollGrab);
        setColour (ScrollBar::trackColourId,                    theme::sunken);

        setColour (AlertWindow::backgroundColourId,             theme::raised);
        setColour (AlertWindow::textColourId,                   theme::text);
        setColour (AlertWindow::outlineColourId,                theme::rule);
    }

    //==========================================================================
    Font HouseLookAndFeel::getTextButtonFont (TextButton&, int)       { return theme::bodyFont(); }

    int HouseLookAndFeel::getTextButtonWidthToFitText (TextButton& b, int buttonHeight)
    {
        // buttonHeight is the bezel's; the box adds the ring's margin.
        const auto textWidth = GlyphArrangement::getStringWidthInt (getTextButtonFont (b, buttonHeight), b.getButtonText());
        return jmax (buttonHeight, textWidth + 2 * theme::buttonPadding) + 2 * theme::ringMargin;
    }

    void HouseLookAndFeel::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool highlighted, bool down)
    {
        const auto on = b.getToggleState();
        auto fill = on ? theme::accent : theme::control;

        if (down)             fill = on ? theme::accentHeld  : theme::controlHeld;
        else if (highlighted) fill = on ? theme::accentHover : theme::controlHover;

        const auto area = b.getLocalBounds().toFloat().reduced ((float) theme::ringMargin);
        g.setColour (fill);
        g.fillRoundedRectangle (area, theme::cornerRadius);

        if (b.hasKeyboardFocus (false))
            drawFocusRing (g, area, theme::cornerRadius);
    }

    void HouseLookAndFeel::drawButtonText (Graphics& g, TextButton& b, bool, bool)
    {
        const auto font = getTextButtonFont (b, b.getHeight());
        g.setFont (font);
        g.setColour (theme::ink);

        const auto area = b.getLocalBounds();
        g.drawText (b.getButtonText(), area, Justification::centred, false);

        // Chosen is shown by shape too, not by the accent alone.
        if (b.getToggleState())
        {
            const auto w = (float) GlyphArrangement::getStringWidth (font, b.getButtonText());
            const auto baseline = (float) area.getCentreY() + font.getHeight() * 0.5f + 1.0f;
            g.fillRect (Rectangle<float> (w, 2.0f).withCentre ({ (float) area.getCentreX(), baseline + 1.0f }));
        }
    }

    void HouseLookAndFeel::drawFocusRing (Graphics& g, Rectangle<float> area, float radius)
    {
        g.setColour (theme::focusRing);
        const auto w = (float) theme::focusRingWidth;
        g.drawRoundedRectangle (area.expanded (w * 0.5f + 0.5f), radius + w * 0.5f, w);
    }

    //==========================================================================
    Font HouseLookAndFeel::getPopupMenuFont()                          { return theme::bodyFont(); }

    void HouseLookAndFeel::drawPopupMenuBackgroundWithOptions (Graphics& g, int w, int h, const PopupMenu::Options&)
    {
        g.fillAll (theme::raised);
        g.setColour (theme::rule);
        g.drawRect (0, 0, w, h, 1);
    }

    Font HouseLookAndFeel::getMenuBarFont (MenuBarComponent&, int, const String&)   { return theme::bodyFont(); }

    void HouseLookAndFeel::drawMenuBarBackground (Graphics& g, int w, int h, bool, MenuBarComponent&)
    {
        g.fillAll (theme::ground);
        g.setColour (theme::rule);
        g.fillRect (0, h - 1, w, 1);
    }

    void HouseLookAndFeel::drawMenuBarItem (Graphics& g, int w, int h, int, const String& text,
                                            bool isMouseOverItem, bool isMenuOpen, bool, MenuBarComponent& bar)
    {
        if (isMenuOpen || isMouseOverItem)
        {
            g.setColour (theme::raised);
            g.fillRect (0, 0, w, h - 1);
        }

        g.setColour (theme::text);
        g.setFont (getMenuBarFont (bar, 0, text));
        g.drawFittedText (text, 0, 0, w, h, Justification::centred, 1);
    }

    //==========================================================================
    String HouseLookAndFeel::wrapTooltip (const String& text)
    {
        StringArray lines;

        for (auto& paragraph : StringArray::fromLines (text))
        {
            String line;

            for (auto& word : StringArray::fromTokens (paragraph, " ", ""))
            {
                if (line.isNotEmpty() && line.length() + 1 + word.length() > theme::tooltipColumns)
                {
                    lines.add (line);
                    line.clear();
                }

                line << (line.isEmpty() ? "" : " ") << word;
            }

            lines.add (line);
        }

        return lines.joinIntoString ("\n");
    }

    static TextLayout layoutTip (const String& text)
    {
        AttributedString s;
        s.setJustification (Justification::topLeft);
        s.append (text, theme::bodyFont(), theme::text);

        TextLayout layout;
        layout.createLayoutWithBalancedLineLengths (s, 10000.0f);
        return layout;
    }

    Rectangle<int> HouseLookAndFeel::getTooltipBounds (const String& tipText, Point<int> screenPos, Rectangle<int> parentArea)
    {
        const auto layout = layoutTip (tipText);
        const auto w = (int) std::ceil (layout.getWidth())  + 20;
        const auto h = (int) std::ceil (layout.getHeight()) + 12;

        return Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                               screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6)  : screenPos.y + 6,
                               w, h).constrainedWithin (parentArea);
    }

    void HouseLookAndFeel::drawTooltip (Graphics& g, const String& text, int width, int height)
    {
        const auto bounds = Rectangle<int> (width, height).toFloat();
        g.setColour (theme::raised);
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (theme::rule);
        g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);

        layoutTip (text).draw (g, bounds.reduced (10.0f, 6.0f));
    }

    //==========================================================================
    void HouseLookAndFeel::drawScrollbar (Graphics& g, ScrollBar&, int x, int y, int w, int h, bool vertical,
                                          int thumbStart, int thumbSize, bool mouseOver, bool mouseDown)
    {
        g.setColour (theme::sunken);
        g.fillRect (x, y, w, h);

        const auto thumb = vertical ? Rectangle<int> (x, thumbStart, w, thumbSize)
                                    : Rectangle<int> (thumbStart, y, thumbSize, h);

        g.setColour (mouseDown ? theme::control : (mouseOver ? theme::edge : theme::scrollGrab));
        g.fillRoundedRectangle (thumb.toFloat().reduced (2.0f), 3.0f);
    }

    void HouseLookAndFeel::drawCornerResizer (Graphics& g, int w, int h, bool mouseOver, bool dragging)
    {
        g.setColour (dragging || mouseOver ? theme::control : theme::edge);

        for (float i = 0.3f; i < 1.0f; i += 0.3f)
            g.drawLine ((float) w * i, (float) h, (float) w, (float) h * i, 1.5f);
    }

    //==========================================================================
    HouseButton::HouseButton (const String& label, const String& tip)
        : TextButton (label)
    {
        setMouseClickGrabsKeyboardFocus (false);
        setWantsKeyboardFocus (true);
        setTip (tip);
    }

    void HouseButton::setTip (const String& tip)
    {
        setTooltip (HouseLookAndFeel::wrapTooltip (tip));
        setDescription (tip);
    }

    bool HouseButton::keyPressed (const KeyPress& key)
    {
        if (key == KeyPress::spaceKey && isEnabled())
        {
            triggerClick();
            return true;
        }

        if (key == KeyPress::escapeKey && hasKeyboardFocus (false))
        {
            giveAwayKeyboardFocus();
            return true;
        }

        return TextButton::keyPressed (key);
    }

    int HouseButton::getIdealWidth()
    {
        return getLookAndFeel().getTextButtonWidthToFitText (*this, theme::controlHeight);
    }
}
