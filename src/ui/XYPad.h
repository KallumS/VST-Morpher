#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../MorphMath.h"

namespace morpher
{
    /** The square you drag to mix the four corners.

        The handle follows the pointer directly; a click jumps it there. From
        the keyboard, the arrow keys move it 5% (1% with Shift) and Esc lets
        go of the keyboard. VoiceOver sees it as two sliders, Morph X and
        Morph Y, since there is no two-way slider role.
    */
    class XYPad final : public juce::Component,
                        public juce::SettableTooltipClient
    {
    public:
        XYPad (juce::RangedAudioParameter& x, juce::RangedAudioParameter& y);
        ~XYPad() override;

        float getMorphX() const noexcept   { return xValue; }
        float getMorphY() const noexcept   { return yValue; }

        /** Where a value lands on screen, and back. For drawing and the tests. */
        juce::Point<float> toScreen (float x, float y) const;
        juce::Point<float> toValue (juce::Point<float> local) const;
        juce::Rectangle<float> getSquare() const;

        void paint (juce::Graphics&) override;
        void resized() override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;
        bool keyPressed (const juce::KeyPress&) override;
        void focusGained (FocusChangeType) override    { repaint(); }
        void focusLost (FocusChangeType) override      { repaint(); }

        std::function<void()> onMove;   // after the position changes, for the corner panels

        static constexpr float handleDiameter = 22.0f;

    private:
        class Axis;
        void moveTo (float x, float y);
        void parameterMoved();
        std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

        juce::ParameterAttachment xAttachment, yAttachment;
        float xValue = 0.0f, yValue = 1.0f;
        bool dragging = false;
        std::unique_ptr<Axis> xAxis, yAxis;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XYPad)
    };
}
