#include "XYPad.h"
#include "HouseLookAndFeel.h"
#include "../Theme.h"

namespace morpher
{
    using namespace juce;

    /** One direction of the pad, as VoiceOver and other screen readers see it. */
    class XYPad::Axis final : public Component
    {
    public:
        Axis (XYPad& p, bool across) : pad (p), isX (across)
        {
            setInterceptsMouseClicks (false, false);
            setWantsKeyboardFocus (false);
            setTitle (isX ? "Morph X" : "Morph Y");
            setDescription (isX ? "Left (A and C) to right (B and D)" : "Bottom (C and D) to top (A and B)");
        }

        std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
        {
            struct Value final : public AccessibilityValueInterface
            {
                explicit Value (Axis& a) : axis (a) {}

                bool isReadOnly() const override               { return false; }
                double getCurrentValue() const override        { return axis.isX ? axis.pad.xValue : axis.pad.yValue; }
                String getCurrentValueAsString() const override { return String (roundToInt (getCurrentValue() * 100.0)) + "%"; }

                void setValue (double v) override
                {
                    const auto f = jlimit (0.0f, 1.0f, (float) v);
                    axis.isX ? axis.pad.moveTo (f, axis.pad.yValue) : axis.pad.moveTo (axis.pad.xValue, f);
                }

                void setValueAsString (const String& s) override { setValue (s.retainCharacters ("0123456789.").getDoubleValue() / 100.0); }
                AccessibleValueRange getRange() const override   { return { { 0.0, 1.0 }, 0.05 }; }

                Axis& axis;
            };

            return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::slider, AccessibilityActions{},
                                                           AccessibilityHandler::Interfaces { std::make_unique<Value> (*this) });
        }

        XYPad& pad;
        const bool isX;
    };

    //==========================================================================
    XYPad::XYPad (RangedAudioParameter& x, RangedAudioParameter& y)
        : xAttachment (x, [this] (float v) { xValue = v; parameterMoved(); }),
          yAttachment (y, [this] (float v) { yValue = v; parameterMoved(); })
    {
        setWantsKeyboardFocus (true);
        setMouseClickGrabsKeyboardFocus (false);
        setTitle ("Morph pad");
        setDescription ("Drag to mix the plug-ins in the four corners. Arrow keys move it.");
        setTooltip (HouseLookAndFeel::wrapTooltip ("Drag to mix the four corners: the nearer a corner, the more of it you hear. "
                                                    "The arrow keys move it too, Shift for smaller steps."));

        xAxis = std::make_unique<Axis> (*this, true);
        yAxis = std::make_unique<Axis> (*this, false);
        addAndMakeVisible (*xAxis);
        addAndMakeVisible (*yAxis);

        xAttachment.sendInitialUpdate();
        yAttachment.sendInitialUpdate();
    }

    XYPad::~XYPad() = default;

    std::unique_ptr<AccessibilityHandler> XYPad::createAccessibilityHandler()
    {
        return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
    }

    void XYPad::parameterMoved()
    {
        // From automation or the host; the range is 0 to 1.
        repaint();
        NullCheckedInvocation::invoke (onMove);
    }

    //==========================================================================
    Rectangle<float> XYPad::getSquare() const
    {
        return getLocalBounds().toFloat().reduced ((float) theme::ringMargin);
    }

    static Rectangle<float> travel (Rectangle<float> square)
    {
        return square.reduced (XYPad::handleDiameter * 0.5f + 6.0f);
    }

    Point<float> XYPad::toScreen (float x, float y) const
    {
        const auto t = travel (getSquare());
        return { t.getX() + x * t.getWidth(), t.getBottom() - y * t.getHeight() };
    }

    Point<float> XYPad::toValue (Point<float> p) const
    {
        const auto t = travel (getSquare());
        return { jlimit (0.0f, 1.0f, (p.x - t.getX()) / jmax (1.0f, t.getWidth())),
                 jlimit (0.0f, 1.0f, (t.getBottom() - p.y) / jmax (1.0f, t.getHeight())) };
    }

    void XYPad::resized()
    {
        xAxis->setBounds (getLocalBounds());
        yAxis->setBounds (getLocalBounds());
    }

    //==========================================================================
    void XYPad::paint (Graphics& g)
    {
        const auto square = getSquare();

        g.setColour (theme::padGround);
        g.fillRoundedRectangle (square, theme::cornerRadius);

        // The grid: halves and quarters, quiet.
        const auto t = travel (square);
        g.setColour (theme::rule);

        for (int q = 1; q <= 3; ++q)
        {
            const auto f = (float) q * 0.25f;
            const auto thickness = q == 2 ? 1.5f : 1.0f;
            g.drawLine (t.getX() + f * t.getWidth(), square.getY() + 1.0f, t.getX() + f * t.getWidth(), square.getBottom() - 1.0f, thickness);
            g.drawLine (square.getX() + 1.0f, t.getY() + f * t.getHeight(), square.getRight() - 1.0f, t.getY() + f * t.getHeight(), thickness);
        }

        // An edge you can see: the pad is something you grab (3:1 or more).
        g.setColour (theme::edge);
        g.drawRoundedRectangle (square.reduced (0.5f), theme::cornerRadius, 1.0f);

        // Each corner's letter and how much of it is heard, set just inside
        // the corner so the handle sitting in the corner never covers it.
        // The plug-ins' names are in the panels beside the pad.
        const auto weights = weightsAt (xValue, yValue);
        const auto font = theme::bodyFont();
        g.setFont (font);

        for (int s = 0; s < numSlots; ++s)
        {
            const auto corner = cornerPosition (s);
            const auto point = toScreen (corner[0], corner[1]);
            const auto left = s == cornerA || s == cornerC;
            const auto letter = String (cornerName (s));
            const auto percent = String (roundToInt (weights[(size_t) s] * 100.0f)) + "%";
            const auto letterWidth = (float) GlyphArrangement::getStringWidth (font, letter);
            const auto percentWidth = (float) GlyphArrangement::getStringWidth (font, percent);
            const auto h = font.getHeight();
            const auto offset = handleDiameter * 0.5f + 8.0f;

            auto x = left ? point.x + offset : point.x - offset - letterWidth - 6.0f - percentWidth;
            const auto y = point.y - h * 0.5f;

            g.setColour (theme::text);
            g.drawText (letter, Rectangle<float> (x, y, letterWidth + 1.0f, h), Justification::centredLeft, false);
            x += letterWidth + 6.0f;
            g.setColour (theme::textSecondary);
            g.drawText (percent, Rectangle<float> (x, y, percentWidth + 1.0f, h), Justification::centredLeft, false);
        }

        // The handle: the controls' grey, the accent while it's held.
        const auto centre = toScreen (xValue, yValue);
        const auto handle = Rectangle<float> (handleDiameter, handleDiameter).withCentre (centre);

        g.setColour (dragging ? theme::accent : theme::control);
        g.fillEllipse (handle);
        g.setColour (theme::ink);
        g.drawEllipse (handle.reduced (1.0f), 2.0f);

        if (hasKeyboardFocus (false))
        {
            HouseLookAndFeel::drawFocusRing (g, square, theme::cornerRadius);
            g.setColour (theme::focusRing);
            g.drawEllipse (handle.expanded (3.0f), 2.0f);
        }
    }

    //==========================================================================
    void XYPad::moveTo (float x, float y)
    {
        x = jlimit (0.0f, 1.0f, x);
        y = jlimit (0.0f, 1.0f, y);

        if (! dragging)
        {
            xAttachment.setValueAsCompleteGesture (x);
            yAttachment.setValueAsCompleteGesture (y);
        }
        else
        {
            xAttachment.setValueAsPartOfGesture (x);
            yAttachment.setValueAsPartOfGesture (y);
        }

        xValue = x;
        yValue = y;
        repaint();
        NullCheckedInvocation::invoke (onMove);
    }

    void XYPad::mouseDown (const MouseEvent& e)
    {
        if (! e.mods.isLeftButtonDown())
            return;

        dragging = true;
        xAttachment.beginGesture();
        yAttachment.beginGesture();

        const auto v = toValue (e.position);
        moveTo (v.x, v.y);
    }

    void XYPad::mouseDrag (const MouseEvent& e)
    {
        if (! dragging)
            return;

        const auto v = toValue (e.position);
        moveTo (v.x, v.y);
    }

    void XYPad::mouseUp (const MouseEvent&)
    {
        if (! dragging)
            return;

        dragging = false;
        xAttachment.endGesture();
        yAttachment.endGesture();
        repaint();
    }

    bool XYPad::keyPressed (const KeyPress& key)
    {
        const auto step = key.getModifiers().isShiftDown() ? 0.01f : 0.05f;
        const auto code = key.getKeyCode();

        if (code == KeyPress::leftKey)  { moveTo (xValue - step, yValue); return true; }
        if (code == KeyPress::rightKey) { moveTo (xValue + step, yValue); return true; }
        if (code == KeyPress::upKey)    { moveTo (xValue, yValue + step); return true; }
        if (code == KeyPress::downKey)  { moveTo (xValue, yValue - step); return true; }

        if (code == KeyPress::escapeKey)
        {
            giveAwayKeyboardFocus();
            return true;
        }

        return false;
    }
}
