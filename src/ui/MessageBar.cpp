#include "MessageBar.h"
#include "../Theme.h"

namespace morpher
{
    using namespace juce;

    namespace
    {
        constexpr int minTextWidth = 200;   // below this, the buttons go under the text
    }

    MessageBar::MessageBar()
    {
        dismissButton.setButtonText ("Dismiss");
        dismissButton.setTip ("Clear this message.");
        actionButton.setTip ("Put back what was there before.");

        actionButton .onClick = [this] { NullCheckedInvocation::invoke (onAction); };
        dismissButton.onClick = [this] { NullCheckedInvocation::invoke (onDismiss); };

        addChildComponent (actionButton);
        addAndMakeVisible (dismissButton);

        setFocusContainerType (FocusContainerType::focusContainer);
        setTitle ("Message");
    }

    void MessageBar::setMessage (const String& newText, bool isWarning, const String& actionName)
    {
        text = newText;
        warning = isWarning;
        setDescription (text);

        actionButton.setButtonText (actionName);
        actionButton.setVisible (actionName.isNotEmpty());

        resized();
        repaint();

        // Let VoiceOver and the like say it.
        if (auto* handler = getAccessibilityHandler(); handler != nullptr && text.isNotEmpty())
            handler->postAnnouncement (text, AccessibilityHandler::AnnouncementPriority::medium);
    }

    int MessageBar::buttonsWidth() const
    {
        auto& self = const_cast<MessageBar&> (*this);
        auto w = self.dismissButton.getIdealWidth();

        if (actionButton.isVisible())
            w += self.actionButton.getIdealWidth() + theme::buttonBoxGap;

        return w;
    }

    bool MessageBar::buttonsBeside (int width) const
    {
        return width - buttonsWidth() - theme::controlGap >= minTextWidth;
    }

    TextLayout MessageBar::layoutText (int width) const
    {
        AttributedString s;
        s.setJustification (Justification::centredLeft);
        s.setWordWrap (AttributedString::byWord);
        s.append (text, theme::bodyFont(), warning ? theme::warning : theme::text);

        TextLayout layout;
        layout.createLayout (s, (float) jmax (1, width));
        return layout;
    }

    int MessageBar::getIdealHeight (int barWidth) const
    {
        if (! hasMessage())
            return 0;

        const auto width = barWidth - 2 * theme::ringMargin;
        const auto beside = buttonsBeside (width);
        const auto textWidth = beside ? width - buttonsWidth() - theme::controlGap : width;
        const auto textH = (int) std::ceil (layoutText (textWidth).getHeight());

        // Bezels are 28 tall; the boxes overhang by the ring's margin.
        return beside ? jmax (textH, theme::controlHeight) + theme::controlGap
                      : textH + theme::controlGap / 2 + theme::controlHeight + theme::controlGap;
    }

    void MessageBar::resized()
    {
        // The bar is wider than its text by the ring's margin each side, so
        // the buttons' rings aren't clipped.
        const auto m = theme::ringMargin;
        const auto width = getWidth() - 2 * m;
        const auto beside = buttonsBeside (width);

        int x = getWidth();
        const int y = beside ? (getHeight() - theme::controlGap - theme::controlHeight) / 2 - m
                             : getHeight() - theme::controlGap - theme::controlHeight - m;

        for (auto* b : { &dismissButton, &actionButton })
        {
            if (! b->isVisible())
                continue;

            const auto w = b->getIdealWidth();
            x -= w;
            b->setBounds (x, y, w, theme::buttonBox);
            x -= theme::buttonBoxGap;
        }
    }

    void MessageBar::paint (Graphics& g)
    {
        const auto m = (float) theme::ringMargin;
        const auto width = getWidth() - 2 * theme::ringMargin;
        const auto beside = buttonsBeside (width);
        const auto textWidth = beside ? width - buttonsWidth() - theme::controlGap : width;
        const auto layout = layoutText (textWidth);

        const auto textArea = beside ? Rectangle<float> (m, 0.0f, (float) textWidth, (float) (getHeight() - theme::controlGap))
                                     : Rectangle<float> (m, 0.0f, (float) textWidth, layout.getHeight());
        layout.draw (g, textArea);

        g.setColour (theme::rule);
        g.fillRect (theme::ringMargin, getHeight() - 1, width, 1);
    }
}
