#include "SlotPanel.h"
#include "../Theme.h"

namespace morpher
{
    using namespace juce;

    namespace
    {
        constexpr int inset = 12;        // panel edge to its contents
        constexpr int badge = 22;        // the corner's letter
        constexpr int barHeight = 4;

        int lineHeight()    { return roundToInt (theme::bodyFont().getHeight()) + 3; }
        String ellipsis()   { return String (CharPointer_UTF8 ("\xe2\x80\xa6")); }
    }

    SlotPanel::SlotPanel (int slotIndex)
        : slot (slotIndex)
    {
        const String corner (cornerName (slot));

        chooseButton.setTip ("Choose a VST3 plug-in for corner " + corner + ".");
        editButton  .setTip ("Show or hide the plug-in's own window.");
        removeButton.setTip ("Take the plug-in out of corner " + corner + ". The corner then passes the sound through. You can undo this.");

        editButton.setClickingTogglesState (false);

        chooseButton.onClick = [this] { NullCheckedInvocation::invoke (onChoose); };
        editButton  .onClick = [this] { NullCheckedInvocation::invoke (onEdit); };
        removeButton.onClick = [this] { NullCheckedInvocation::invoke (onRemove); };

        for (auto* b : { &chooseButton, &editButton, &removeButton })
            addChildComponent (b);

        setFocusContainerType (FocusContainerType::focusContainer);
        setState ({});
    }

    void SlotPanel::setState (const State& s)
    {
        state = s;
        const String corner (cornerName (slot));
        const auto empty = state.kind == State::Kind::empty;

        chooseButton.setButtonText (empty ? "Choose Plug-in" + ellipsis() : "Replace" + ellipsis());
        chooseButton.setTitle (chooseButton.getButtonText() + " (" + corner + ")");
        editButton.setButtonText ("Edit");
        editButton.setTitle ("Edit " + state.name);
        editButton.setToggleState (state.windowOpen, dontSendNotification);
        removeButton.setButtonText ("Remove");
        removeButton.setTitle ("Remove " + state.name + " from " + corner);

        chooseButton.setVisible (true);
        editButton  .setVisible (state.kind == State::Kind::loaded && state.hasEditor);
        removeButton.setVisible (! empty);

        const auto percent = String (roundToInt (state.level * 100.0f)) + "%";
        setTitle ("Corner " + corner + ": " + (empty ? String ("empty") : state.name) + ", level " + percent);

        resized();
        repaint();
    }

    Array<HouseButton*> SlotPanel::visibleButtons() const
    {
        Array<HouseButton*> result;

        for (auto* b : { &chooseButton, &editButton, &removeButton })
            if (b->isVisible())
                result.add (const_cast<HouseButton*> (b));

        return result;
    }

    int SlotPanel::layoutButtons (Rectangle<int> area, bool apply)
    {
        // Rows of buttons that wrap at the panel's edge rather than run off it.
        int x = area.getX(), y = area.getY();

        for (auto* b : visibleButtons())
        {
            const auto w = jmin (b->getIdealWidth(), area.getWidth());

            if (x > area.getX() && x + w > area.getRight())
            {
                x = area.getX();
                y += theme::buttonBox + theme::buttonBoxGap;
            }

            if (apply)
                b->setBounds (x, y, w, theme::buttonBox);

            x += w + theme::buttonBoxGap;
        }

        return y + theme::buttonBox - area.getY();
    }

    int SlotPanel::textHeight (int) const
    {
        // Name and level, then what it is, then the level's bar.
        return jmax (badge, lineHeight()) + 2 + lineHeight() + 4 + barHeight;
    }

    Rectangle<int> SlotPanel::buttonArea (int width, int height) const
    {
        // The buttons' boxes overhang by the ring's margin, so their bezels
        // line up with the text above them.
        const auto top = inset + textHeight (width) + 8 - theme::ringMargin;
        return { inset - theme::ringMargin, top, width - 2 * (inset - theme::ringMargin), jmax (0, height - top) };
    }

    int SlotPanel::getIdealHeight (int width) const
    {
        auto* self = const_cast<SlotPanel*> (this);
        const auto buttons = self->layoutButtons (buttonArea (width, 10000), false);
        return inset + textHeight (width) + 8 + buttons - 2 * theme::ringMargin + inset;
    }

    void SlotPanel::resized()
    {
        layoutButtons (buttonArea (getWidth(), getHeight()), true);
    }

    void SlotPanel::paint (Graphics& g)
    {
        const auto bounds = getLocalBounds().toFloat();
        g.setColour (theme::sunken);
        g.fillRoundedRectangle (bounds, theme::cornerRadius);

        auto inner = getLocalBounds().reduced (inset);
        const auto font = theme::bodyFont();
        g.setFont (font);

        // The corner's letter.
        auto row = inner.removeFromTop (jmax (badge, lineHeight()));
        const auto badgeArea = row.removeFromLeft (badge).withSizeKeepingCentre (badge, badge).toFloat();
        g.setColour (theme::rule);
        g.fillRoundedRectangle (badgeArea, 4.0f);
        g.setColour (theme::text);
        g.drawText (cornerName (slot), badgeArea, Justification::centred, false);
        row.removeFromLeft (8);

        // How much of it is heard, in words; the bar below says it again.
        const auto percent = String (roundToInt (state.level * 100.0f)) + "%";
        const auto percentWidth = GlyphArrangement::getStringWidthInt (font, "100%");
        g.setColour (theme::textSecondary);
        g.drawText (percent, row.removeFromRight (percentWidth), Justification::centredRight, false);
        row.removeFromRight (8);

        const auto empty = state.kind == State::Kind::empty;
        g.setColour (theme::text);
        g.drawFittedText (empty ? String ("Empty") : state.name, row, Justification::centredLeft, 1, 1.0f);

        // What it is, or why it isn't there.
        inner.removeFromTop (2);
        const auto detailRow = inner.removeFromTop (lineHeight());

        if (state.kind == State::Kind::missing)
        {
            g.setColour (theme::warning);
            g.drawFittedText ("Not found. Its settings are kept.", detailRow, Justification::centredLeft, 1, 1.0f);
        }
        else
        {
            g.setColour (theme::textSecondary);
            g.drawFittedText (empty ? String ("Passes the sound through") : state.detail,
                              detailRow, Justification::centredLeft, 1, 1.0f);
        }

        inner.removeFromTop (4);
        const auto bar = inner.removeFromTop (barHeight).toFloat();
        g.setColour (theme::padGround);
        g.fillRoundedRectangle (bar, 2.0f);
        g.setColour (theme::control);
        g.fillRoundedRectangle (bar.withWidth (bar.getWidth() * jlimit (0.0f, 1.0f, state.level)), 2.0f);
    }
}
