#include "PluginWindow.h"
#include "../Theme.h"

namespace morpher
{
    using namespace juce;

    PluginWindow::PluginWindow (AudioPluginInstance& plugin, const String& title, std::function<void()> onClose)
        : DocumentWindow (title, theme::ground, DocumentWindow::closeButton | DocumentWindow::minimiseButton),
          closed (std::move (onClose))
    {
        setUsingNativeTitleBar (true);

        auto* editor = plugin.hasEditor() ? plugin.createEditorAndMakeActive() : nullptr;

        if (editor == nullptr)
            editor = new GenericAudioProcessorEditor (plugin);

        setContentOwned (editor, true);
        setResizable (editor->isResizable(), false);

        // A window from inside a plug-in otherwise falls behind the DAW's own.
        setAlwaysOnTop (true);
        centreWithSize (getWidth(), getHeight());
        setVisible (true);
    }

    PluginWindow::~PluginWindow()
    {
        clearContentComponent();
    }

    void PluginWindow::closeButtonPressed()
    {
        // The owner deletes this window; do nothing after the call.
        NullCheckedInvocation::invoke (closed);
    }
}
