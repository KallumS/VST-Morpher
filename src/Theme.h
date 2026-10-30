#pragma once

#include <juce_graphics/juce_graphics.h>

/** The house scheme (docs/COLOUR.md) and the sizes from Apple's guidelines
    (docs/apple-hig-review.md). Colours are named by role; nothing else in the
    code writes a colour value. If you change one here, change COLOUR.md too.
*/
namespace morpher::theme
{
    using juce::Colour;

    // The four that carry it.
    inline const Colour accent  { 0xffFFF200 };   // whatever is switched on
    inline const Colour control { 0xffA9AFBA };   // buttons and the pad's handle, raised off the ground
    inline const Colour ground  { 0xff23272E };   // the window behind everything
    inline const Colour ink     { 0xff14171C };   // the text on any button

    // Control states. The house scripts derive the accent's two with shade(),
    // 18% toward white and black; these are its results, stored.
    inline const Colour controlHover { 0xffC0C6CF };
    inline const Colour controlHeld  { 0xff8F96A2 };
    inline const Colour accentHover  { 0xffFFF42E };
    inline const Colour accentHeld   { 0xffD1C600 };

    // The ramp's structural greys.
    inline const Colour padGround  { 0xff111419 };   // the pad, like the house piano roll
    inline const Colour sunken     { 0xff1A1D23 };   // the four slot panels
    inline const Colour rule       { 0xff3A404A };   // separators, the pad's grid, raised edges
    inline const Colour raised     { 0xff2A2F37 };   // menus and tooltips: lighter than the window
    inline const Colour edge       { 0xff8A919C };   // edges of things you can see and grab: 4.7:1
    inline const Colour scrollGrab { 0xff6D7581 };

    // Words.
    inline const Colour text          { 0xffDDE1E7 };   // body text
    inline const Colour textSecondary { 0xffB0B6C0 };   // 7.4:1 on the ground
    inline const Colour textOnRaised  { 0xffBFC5CE };   // secondary words on menus: 7.8:1
    inline const Colour warning       { 0xffFF9E96 };   // 7.6:1 on the ground; never the accent's hue
    inline const Colour focusRing     { 0xffF2F4F7 };   // near white, never the accent

    //==========================================================================
    // Sizes, in points at 100%. Text Size multiplies all of them.
    inline constexpr float bodyPoints     = 13.0f;   // the Mac's default text size
    inline constexpr int   controlHeight  = 28;      // the Mac's default control height
    inline constexpr int   controlGap     = 12;      // between controls with a bezel
    inline constexpr int   margin         = 16;      // window edge to content
    inline constexpr int   buttonPadding  = 12;      // label to button edge, each side
    inline constexpr int   menuBarHeight  = 24;
    inline constexpr float cornerRadius   = 6.0f;
    inline constexpr int   focusRingWidth = 2;
    inline constexpr int   tooltipColumns = 64;      // characters per tooltip line

    // A button's component is its bezel plus room for the focus ring all
    // round, so laying out boxes uses these; the bezels end up 28 tall and
    // 12 apart.
    inline constexpr int ringMargin   = focusRingWidth + 1;
    inline constexpr int buttonBox    = controlHeight + 2 * ringMargin;
    inline constexpr int buttonBoxGap = controlGap - 2 * ringMargin;

    inline constexpr int textSizes[] { 100, 125, 150, 200 };

    //==========================================================================
    /** WCAG relative luminance and contrast ratio, as Apple's guidelines use. */
    inline double luminance (Colour c)
    {
        auto lin = [] (juce::uint8 v)
        {
            const auto s = v / 255.0;
            return s <= 0.04045 ? s / 12.92 : std::pow ((s + 0.055) / 1.055, 2.4);
        };

        return 0.2126 * lin (c.getRed()) + 0.7152 * lin (c.getGreen()) + 0.0722 * lin (c.getBlue());
    }

    inline double contrast (Colour a, Colour b)
    {
        auto la = luminance (a), lb = luminance (b);
        if (la < lb) std::swap (la, lb);
        return (la + 0.05) / (lb + 0.05);
    }

    /** The body font at a text size (1.0 = 100%). */
    inline juce::Font bodyFont (float scale = 1.0f)
    {
        return juce::Font (juce::FontOptions().withPointHeight (bodyPoints * scale));
    }
}
