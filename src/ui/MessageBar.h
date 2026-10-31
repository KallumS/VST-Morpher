#pragma once

#include "HouseLookAndFeel.h"

namespace morpher
{
    /** The line at the top of the window that says what just happened or what
        went wrong. It stays until Dismiss, or until a newer message replaces
        it; nothing times out. An Undo sits beside a message that can be undone.
    */
    class MessageBar final : public juce::Component
    {
    public:
        MessageBar();

        void setMessage (const juce::String& text, bool isWarning, const juce::String& actionName);
        bool hasMessage() const noexcept      { return text.isNotEmpty(); }
        const juce::String& getText() const   { return text; }

        int getIdealHeight (int width) const;

        void paint (juce::Graphics&) override;
        void resized() override;

        std::function<void()> onAction, onDismiss;

        HouseButton actionButton, dismissButton;

    private:
        int buttonsWidth() const;
        bool buttonsBeside (int width) const;
        juce::TextLayout layoutText (int width) const;

        juce::String text;
        bool warning = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessageBar)
    };
}
