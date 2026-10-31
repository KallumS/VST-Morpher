#pragma once

#include "../MorpherProcessor.h"
#include "../Settings.h"
#include "HouseLookAndFeel.h"
#include "MessageBar.h"
#include "PluginWindow.h"
#include "SlotPanel.h"
#include "XYPad.h"

namespace morpher
{
    /** Morpher's window: the View menu, the message line, the pad, and a
        panel for each corner placed beside the corner it stands for.

        Everything is laid out in points at 100% inside one content
        component; Text Size scales that component, so words, controls and
        the window grow together and nothing is cropped (decision 0008).
    */
    class MorpherEditor final : public juce::AudioProcessorEditor,
                                private juce::ChangeListener,
                                private MorpherProcessor::SlotListener,
                                private juce::MenuBarModel
    {
    public:
        explicit MorpherEditor (MorpherProcessor&);
        ~MorpherEditor() override;

        void paint (juce::Graphics&) override;
        void resized() override;
        bool keyPressed (const juce::KeyPress&) override;

        //==========================================================================
        // Text Size.
        int getTextSize() const noexcept           { return textSize; }
        void setTextSize (int percent);
        float getScale() const noexcept           { return (float) textSize / 100.0f; }

        static constexpr int defaultWidth = 600, defaultHeight = 620;   // points at 100%
        static constexpr int minWidth = 480, minHeight = 560;
        static constexpr int wideFrom = 980;    // from here, the panels go beside the pad rather than above and below

        //==========================================================================
        // For the tests.
        XYPad& getPad()                            { return pad; }
        SlotPanel& getPanel (int s)                { return *panels[(size_t) s]; }
        MessageBar& getMessageBar()                { return messageBar; }
        juce::Component& getContent()             { return content; }
        bool isWindowOpen (int s) const            { return windows[(size_t) s] != nullptr; }
        void toggleWindow (int slot);
        void chooseFile (int slot, const juce::File&);
        juce::PopupMenu createChooseMenu (int slot);

    private:
        struct Content final : public juce::Component
        {
            std::function<void()> onResize;
            void resized() override  { juce::NullCheckedInvocation::invoke (onResize); }
        };

        void layout();
        void refresh();
        void updateLevels();
        void showChooseMenu (int slot);
        void browse (int slot);
        void loadChosen (int slot, const juce::PluginDescription&);
        void closeWindow (int slot);
        static juce::Array<juce::File> findInstalledPlugins();

        void changeListenerCallback (juce::ChangeBroadcaster*) override;
        void slotWillChange (int slot) override;

        juce::StringArray getMenuBarNames() override;
        juce::PopupMenu getMenuForIndex (int, const juce::String&) override;
        void menuItemSelected (int, int) override {}

        MorpherProcessor& processor;
        juce::SharedResourcePointer<Settings> settings;
        HouseLookAndFeel lookAndFeel;

        Content content;
        juce::MenuBarComponent menuBar;
        MessageBar messageBar;
        XYPad pad;
        std::array<std::unique_ptr<SlotPanel>, numSlots> panels;
        std::array<std::unique_ptr<PluginWindow>, numSlots> windows;

        juce::TooltipWindow tooltips { this, 700 };
        std::unique_ptr<juce::FileChooser> chooser;
        int textSize = 100;
        juce::Point<int> logicalSize { defaultWidth, defaultHeight };
        bool settingSize = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MorpherEditor)
    };
}
