#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"
#include "ui/HouseLookAndFeel.h"

using namespace morpher;

struct ThemeTests final : juce::UnitTest
{
    ThemeTests() : juce::UnitTest ("Colour and type (Apple's guidelines)", "Morpher") {}

    void expectContrast (juce::Colour a, juce::Colour b, double atLeast, const juce::String& what)
    {
        const auto c = theme::contrast (a, b);
        expect (c >= atLeast, what + ": " + juce::String (c, 2) + ":1, needs " + juce::String (atLeast, 1) + ":1");
    }

    void runTest() override
    {
        using namespace theme;

        beginTest ("Every line of text is 7:1 on what it sits on");
        expectContrast (text,          ground,    7.0, "body text on the window");
        expectContrast (textSecondary, ground,    7.0, "secondary text on the window");
        expectContrast (warning,       ground,    7.0, "a warning on the window");
        expectContrast (text,          sunken,    7.0, "body text on a corner's panel");
        expectContrast (textSecondary, sunken,    7.0, "secondary text on a corner's panel");
        expectContrast (warning,       sunken,    7.0, "a warning on a corner's panel");
        expectContrast (text,          padGround, 7.0, "a corner's name on the pad");
        expectContrast (textSecondary, padGround, 7.0, "a corner's level on the pad");
        expectContrast (text,          rule,      7.0, "a corner's letter on its badge");
        expectContrast (text,          raised,    7.0, "menu and tooltip text");
        expectContrast (textOnRaised,  raised,    7.0, "a menu's headings and keys");
        expectContrast (text,          rule,      7.0, "a highlighted menu item");

        beginTest ("Every button's ink is 7:1 on its fill, at rest, hovered and chosen");
        for (auto fill : { control, controlHover, accent, accentHover, accentHeld })
            expectContrast (ink, fill, 7.0, "ink on " + fill.toDisplayString (false));

        // Held lasts only while the button is down; Apple's minimum for 13 pt text is 4.5:1.
        expectContrast (ink, controlHeld, 4.5, "ink on a held button");

        beginTest ("Things you can see and grab are 3:1 or more");
        expectContrast (edge,       ground,    3.0, "the pad's edge on the window");
        expectContrast (edge,       padGround, 3.0, "the pad's edge on the pad");
        expectContrast (control,    padGround, 3.0, "the handle on the pad");
        expectContrast (accent,     padGround, 3.0, "the held handle on the pad");
        expectContrast (control,    sunken,    3.0, "the level bar on its panel");
        expectContrast (control,    ground,    3.0, "a button on the window");
        expectContrast (control,    sunken,    3.0, "a button on a panel");
        expectContrast (scrollGrab, sunken,    3.0, "a scrollbar's thumb");
        expectContrast (focusRing,  ground,    3.0, "the focus ring on the window");
        expectContrast (focusRing,  sunken,    3.0, "the focus ring on a panel");
        expectContrast (focusRing,  padGround, 3.0, "the focus ring on the pad");

        beginTest ("Menus and tooltips are lighter than the window");
        expect (luminance (raised) > luminance (ground));

        beginTest ("The focus ring is not the accent");
        expect (focusRing != accent);
        expect (std::abs (focusRing.getHue() - accent.getHue()) > 0.05f || focusRing.getSaturation() < 0.1f);

        beginTest ("The warning colour isn't a shade of the accent");
        expect (std::abs (warning.getHue() - accent.getHue()) > 0.08f);

        beginTest ("Keep the greys blue: R < G < B");
        for (auto c : { ground, padGround, sunken, rule, raised, edge, scrollGrab, control, controlHover,
                        controlHeld, ink, text, textSecondary, textOnRaised, focusRing })
            expect (c.getRed() < c.getGreen() && c.getGreen() < c.getBlue(), c.toDisplayString (false) + " isn't blue");

        beginTest ("The house values");
        expect (accent  == juce::Colour (0xffFFF200));
        expect (control == juce::Colour (0xffA9AFBA));
        expect (ground  == juce::Colour (0xff23272E));
        expect (ink     == juce::Colour (0xff14171C));
        expect (accentHover == juce::Colour (0xffFFF42E), accentHover.toDisplayString (false));
        expect (accentHeld  == juce::Colour (0xffD1C600), accentHeld.toDisplayString (false));

        beginTest ("Type: 13 points, regular weight; controls 28 tall and 12 apart");
        expectWithinAbsoluteError (bodyFont().getHeightInPoints(), 13.0f, 0.01f);
        expect (! bodyFont().isBold());
        expectEquals (controlHeight, 28);
        expectEquals (controlGap, 12);
        expectEquals (buttonBox - 2 * ringMargin, controlHeight);
        expectEquals (buttonBoxGap + 2 * ringMargin, controlGap);

        beginTest ("Tooltips are broken into lines of about 64 characters");
        const juce::String longTip = "Every corner runs all the time, heard or not, so its tail and timing are right "
                                     "the moment you move towards it, and the sound never jumps when you start to drag.";
        for (auto& line : juce::StringArray::fromLines (HouseLookAndFeel::wrapTooltip (longTip)))
            expect (line.length() <= tooltipColumns, line);
        expectEquals (HouseLookAndFeel::wrapTooltip (longTip).removeCharacters ("\n ").length(),
                      longTip.removeCharacters (" ").length());
    }
};

static ThemeTests themeTests;
