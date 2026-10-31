#pragma once

#include "HouseLookAndFeel.h"
#include "../MorphMath.h"

namespace morpher
{
    /** One corner's panel: what's in it, how much of it is heard, and its
        buttons. Buttons that would do nothing aren't shown (an empty corner
        has no Edit or Remove), rather than shown dimmed.
    */
    class SlotPanel final : public juce::Component
    {
    public:
        explicit SlotPanel (int slotIndex);

        struct State
        {
            enum class Kind { empty, loaded, missing } kind = Kind::empty;
            juce::String name, detail;
            float level = 0.0f;           // 0 to 1
            bool windowOpen = false;
            bool hasEditor = false;
        };

        void setState (const State&);
        const State& getState() const noexcept   { return state; }

        /** The height this panel needs at a width, buttons wrapped. */
        int getIdealHeight (int width) const;

        void paint (juce::Graphics&) override;
        void resized() override;

        std::function<void()> onChoose, onEdit, onRemove;

        HouseButton chooseButton, editButton, removeButton;

        const int slot;

    private:
        juce::Array<HouseButton*> visibleButtons() const;
        int layoutButtons (juce::Rectangle<int> area, bool apply);
        juce::Rectangle<int> buttonArea (int width, int height) const;
        int textHeight (int width) const;

        State state;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlotPanel)
    };
}
