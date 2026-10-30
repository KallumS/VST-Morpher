#include "MorpherEditor.h"
#include "../Theme.h"

#include <map>

namespace morpher
{
    using namespace juce;

    namespace
    {
        String ellipsis()   { return String (CharPointer_UTF8 ("\xe2\x80\xa6")); }

        String describe (const PluginDescription& d)
        {
            return d.manufacturerName.isNotEmpty() ? d.manufacturerName + ", " + d.pluginFormatName
                                                   : d.pluginFormatName;
        }
    }

    MorpherEditor::MorpherEditor (MorpherProcessor& p)
        : AudioProcessorEditor (p),
          processor (p),
          pad (p.getXParameter(), p.getYParameter())
    {
        setLookAndFeel (&lookAndFeel);
        setTitle ("Morpher");
        setWantsKeyboardFocus (true);
        setFocusContainerType (FocusContainerType::keyboardFocusContainer);

        addAndMakeVisible (content);
        content.onResize = [this] { layout(); };

        menuBar.setModel (this);
        content.addAndMakeVisible (menuBar);
        content.addChildComponent (messageBar);
        content.addAndMakeVisible (pad);

        for (int s = 0; s < numSlots; ++s)
        {
            auto panel = std::make_unique<SlotPanel> (s);
            panel->onChoose = [this, s] { showChooseMenu (s); };
            panel->onEdit   = [this, s] { toggleWindow (s); };
            panel->onRemove = [this, s] { processor.removeWithUndo (s); };
            content.addAndMakeVisible (*panel);
            panels[(size_t) s] = std::move (panel);
        }

        // Tab goes: the message, the pad, then the corners in reading order.
        messageBar.setExplicitFocusOrder (1);
        pad.setExplicitFocusOrder (2);

        for (int s = 0; s < numSlots; ++s)
            panels[(size_t) s]->setExplicitFocusOrder (3 + s);

        messageBar.onAction  = [this] { processor.runNoticeAction(); };
        messageBar.onDismiss = [this] { processor.dismissNotice(); };
        pad.onMove = [this] { updateLevels(); };

        processor.changes.addChangeListener (this);
        processor.addSlotListener (this);

        textSize = settings->getTextSize();
        setResizable (true, true);
        setTextSize (textSize);
        refresh();
    }

    MorpherEditor::~MorpherEditor()
    {
        for (auto& w : windows)
            w.reset();

        processor.removeSlotListener (this);
        processor.changes.removeChangeListener (this);
        menuBar.setModel (nullptr);
        setLookAndFeel (nullptr);
    }

    //==========================================================================
    void MorpherEditor::setTextSize (int percent)
    {
        textSize = Settings::clampTextSize (percent);
        settings->setTextSize (textSize);

        const auto s = getScale();
        const ScopedValueSetter<bool> svs (settingSize, true);

        setResizeLimits (roundToInt (minWidth * s), roundToInt (minHeight * s), 8000, 8000);
        setSize (roundToInt ((float) logicalSize.x * s), roundToInt ((float) logicalSize.y * s));
        resized();
        menuItemsChanged();
    }

    void MorpherEditor::resized()
    {
        const auto s = getScale();

        if (! settingSize)
            logicalSize = { jmax (minWidth,  roundToInt ((float) getWidth()  / s)),
                            jmax (minHeight, roundToInt ((float) getHeight() / s)) };

        content.setTransform (AffineTransform::scale (s));
        content.setBounds (0, 0, roundToInt ((float) getWidth() / s), roundToInt ((float) getHeight() / s));
    }

    void MorpherEditor::paint (Graphics& g)
    {
        g.fillAll (theme::ground);
    }

    void MorpherEditor::layout()
    {
        auto area = content.getLocalBounds();
        const auto gap = theme::controlGap;
        const auto m = theme::ringMargin;

        menuBar.setBounds (area.removeFromTop (theme::menuBarHeight));
        area.reduce (theme::margin, 0);
        area.removeFromTop (gap);
        area.removeFromBottom (theme::margin);

        // The message goes at the top, never at the window's bottom edge.
        if (messageBar.hasMessage())
        {
            const auto h = messageBar.getIdealHeight (area.getWidth() + 2 * m);
            messageBar.setBounds (area.removeFromTop (h).expanded (m, 0));
            messageBar.setVisible (true);
            area.removeFromTop (gap);
        }
        else
        {
            messageBar.setVisible (false);
        }

        auto& a = *panels[cornerA];
        auto& b = *panels[cornerB];
        auto& c = *panels[cornerC];
        auto& d = *panels[cornerD];

        if (content.getWidth() >= wideFrom)
        {
            // Each corner's panel beside its corner of the pad, the pad as
            // near square as the panels allow.
            const auto column = jlimit (260, 340, (area.getWidth() - 2 * gap - area.getHeight()) / 2);
            auto left  = area.removeFromLeft (column);
            auto right = area.removeFromRight (column);
            area.reduce (gap, 0);

            a.setBounds (left .removeFromTop    (a.getIdealHeight (column)));
            c.setBounds (left .removeFromBottom (c.getIdealHeight (column)));
            b.setBounds (right.removeFromTop    (b.getIdealHeight (column)));
            d.setBounds (right.removeFromBottom (d.getIdealHeight (column)));
        }
        else
        {
            // A and B above the pad, C and D below it.
            const auto column = (area.getWidth() - gap) / 2;
            const auto topH    = jmax (a.getIdealHeight (column), b.getIdealHeight (column));
            const auto bottomH = jmax (c.getIdealHeight (column), d.getIdealHeight (column));

            auto top = area.removeFromTop (topH);
            a.setBounds (top.removeFromLeft (column));
            b.setBounds (top.removeFromRight (column));
            area.removeFromTop (gap);

            auto bottom = area.removeFromBottom (bottomH);
            c.setBounds (bottom.removeFromLeft (column));
            d.setBounds (bottom.removeFromRight (column));
            area.removeFromBottom (gap);
        }

        pad.setBounds (area.expanded (m));
    }

    //==========================================================================
    void MorpherEditor::changeListenerCallback (ChangeBroadcaster*)
    {
        refresh();
    }

    void MorpherEditor::refresh()
    {
        for (int s = 0; s < numSlots; ++s)
        {
            SlotPanel::State state;
            const auto status = processor.getStatus (s);
            const auto desc = processor.getDescription (s);

            if (status == MorpherProcessor::SlotStatus::loaded)
            {
                state.kind = SlotPanel::State::Kind::loaded;
                state.detail = describe (desc);
            }
            else if (status == MorpherProcessor::SlotStatus::missing)
            {
                state.kind = SlotPanel::State::Kind::missing;
            }

            state.name = processor.getPluginName (s);
            state.windowOpen = windows[(size_t) s] != nullptr;
            state.hasEditor = true;     // one without its own gets a plain one
            state.level = panels[(size_t) s]->getState().level;
            panels[(size_t) s]->setState (state);
        }

        const auto& notice = processor.getNotice();
        messageBar.setMessage (notice.text, notice.isWarning, notice.actionName);

        updateLevels();
        layout();
    }

    void MorpherEditor::updateLevels()
    {
        const auto w = weightsAt (pad.getMorphX(), pad.getMorphY());

        for (int s = 0; s < numSlots; ++s)
        {
            auto state = panels[(size_t) s]->getState();

            if (std::abs (state.level - w[(size_t) s]) > 0.0001f)
            {
                state.level = w[(size_t) s];
                panels[(size_t) s]->setState (state);
            }
        }
    }

    //==========================================================================
    Array<File> MorpherEditor::findInstalledPlugins()
    {
        // Only file names: nothing is opened until you choose one, so a
        // broken plug-in elsewhere can't take the DAW down (decision 0002).
        Array<File> found;
        VST3PluginFormat vst3;
        const auto search = vst3.getDefaultLocationsToSearch();

        std::function<void (const File&, int)> walk = [&] (const File& dir, int depth)
        {
            for (const auto& entry : RangedDirectoryIterator (dir, false, "*", File::findFilesAndDirectories))
            {
                const auto f = entry.getFile();

                if (f.hasFileExtension ("vst3"))
                    found.addIfNotAlreadyThere (f);
                else if (entry.isDirectory() && depth < 2)
                    walk (f, depth + 1);
            }
        };

        for (int i = 0; i < search.getNumPaths(); ++i)
            if (search[i].isDirectory())
                walk (search[i], 0);

        std::sort (found.begin(), found.end(), [] (const File& x, const File& y)
        {
            return x.getFileNameWithoutExtension().compareNatural (y.getFileNameWithoutExtension()) < 0;
        });

        return found;
    }

    PopupMenu MorpherEditor::createChooseMenu (int slot)
    {
        PopupMenu menu;
        const auto recent = settings->getRecentPlugins();

        if (! recent.isEmpty())
        {
            menu.addSectionHeader ("Recent");

            for (const auto& d : recent)
                menu.addItem (d.name + "  (" + describe (d) + ")", [this, slot, d] { loadChosen (slot, d); });

            menu.addSeparator();
        }

        PopupMenu installed;
        std::map<String, PopupMenu> byFolder;
        const auto files = findInstalledPlugins();

        for (const auto& f : files)
        {
            auto choose = [this, slot, f] { chooseFile (slot, f); };
            const auto folder = f.getParentDirectory();

            // Plug-ins in a maker's folder are grouped under it.
            if (folder.getFileName().equalsIgnoreCase ("VST3") || folder.getFileName().equalsIgnoreCase (".vst3"))
                installed.addItem (f.getFileNameWithoutExtension(), choose);
            else
                byFolder[folder.getFileName()].addItem (f.getFileNameWithoutExtension(), choose);
        }

        PopupMenu grouped;

        for (auto& [name, sub] : byFolder)
            grouped.addSubMenu (name, sub);

        if (! byFolder.empty() && installed.getNumItems() > 0)
            grouped.addSeparator();

        for (PopupMenu::MenuItemIterator it (installed); it.next();)
            grouped.addItem (it.getItem());

        if (grouped.getNumItems() > 0)
            menu.addSubMenu ("Installed", grouped);

        menu.addItem ("Choose File" + ellipsis(), [this, slot] { browse (slot); });

        if (! recent.isEmpty())
        {
            menu.addSeparator();
            menu.addItem ("Clear Recent", [this] { settings->clearRecentPlugins(); });
        }

        return menu;
    }

    void MorpherEditor::showChooseMenu (int slot)
    {
        auto& anchor = panels[(size_t) slot]->chooseButton;
        createChooseMenu (slot).showMenuAsync (PopupMenu::Options().withTargetComponent (&anchor)
                                                                     .withInitiallySelectedItem (0));
    }

    void MorpherEditor::browse (int slot)
    {
        auto start = settings->getLastFolder();

        if (! start.isDirectory())
        {
            VST3PluginFormat vst3;
            const auto search = vst3.getDefaultLocationsToSearch();

            for (int i = 0; i < search.getNumPaths() && ! start.isDirectory(); ++i)
                start = search[i];
        }

        chooser = std::make_unique<FileChooser> ("Choose a VST3 plug-in for corner " + String (cornerName (slot)),
                                                 start, "*.vst3");

        // On Windows a chooser can pick files or folders but not both; a
        // Windows VST3 is usually a file you can pick inside its folder.
        auto chooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;

       #if ! JUCE_WINDOWS
        chooserFlags |= FileBrowserComponent::canSelectDirectories;
       #endif

        chooser->launchAsync (chooserFlags, [this, slot] (const FileChooser& fc)
        {
            const auto file = fc.getResult();

            if (file != File())
            {
                settings->setLastFolder (file.getParentDirectory());
                chooseFile (slot, file);
            }
        });
    }

    void MorpherEditor::chooseFile (int slot, const File& file)
    {
        auto types = processor.findPluginsIn (file);

        if (types.isEmpty())
        {
            processor.setNotice ({ file.getFileName() + " isn't a VST3 plug-in Morpher can open.", true, {}, {} });
            return;
        }

        if (types.size() == 1)
        {
            loadChosen (slot, *types[0]);
            return;
        }

        // One file, several plug-ins: ask which.
        PopupMenu which;
        which.addSectionHeader (file.getFileNameWithoutExtension());

        for (auto* t : types)
            which.addItem (t->name, [this, slot, d = *t] { loadChosen (slot, d); });

        which.showMenuAsync (PopupMenu::Options().withTargetComponent (&panels[(size_t) slot]->chooseButton));
    }

    void MorpherEditor::loadChosen (int slot, const PluginDescription& d)
    {
        if (processor.loadWithUndo (slot, d).isEmpty())
            settings->addRecentPlugin (processor.getDescription (slot));
    }

    //==========================================================================
    void MorpherEditor::toggleWindow (int slot)
    {
        if (windows[(size_t) slot] != nullptr)
        {
            closeWindow (slot);
            return;
        }

        if (auto* instance = processor.getInstance (slot))
        {
            const auto title = processor.getPluginName (slot) + " (" + cornerName (slot) + ")";
            windows[(size_t) slot] = std::make_unique<PluginWindow> (*instance, title, [this, slot]
            {
                // Closed from its own title bar: let go once its click has finished.
                MessageManager::callAsync ([safe = SafePointer<MorpherEditor> (this), slot]
                {
                    if (safe != nullptr)
                        safe->closeWindow (slot);
                });
            });
        }

        refresh();
    }

    void MorpherEditor::closeWindow (int slot)
    {
        windows[(size_t) slot].reset();
        refresh();
    }

    void MorpherEditor::slotWillChange (int slot)
    {
        // The plug-in is about to go: its window must go first.
        windows[(size_t) slot].reset();
    }

    //==========================================================================
    StringArray MorpherEditor::getMenuBarNames()
    {
        return { "View" };
    }

    PopupMenu MorpherEditor::getMenuForIndex (int, const String&)
    {
        PopupMenu view, sizes;

        for (auto size : theme::textSizes)
            sizes.addItem (PopupMenu::Item (String (size) + "%")
                               .setTicked (size == textSize)
                               .setAction ([this, size] { setTextSize (size); }));

        view.addSubMenu ("Text Size", sizes);
        view.addSeparator();

        auto item = [this] (const String& name, KeyPress key, int target, bool enabled)
        {
            PopupMenu::Item i (name);
            i.shortcutKeyDescription = key.getTextDescriptionWithIcons();
            i.isEnabled = enabled;
            i.action = [this, target] { setTextSize (target); };
            return i;
        };

        int index = 0;
        for (int i = 0; i < (int) std::size (theme::textSizes); ++i)
            if (theme::textSizes[i] == textSize)
                index = i;

        const auto last = (int) std::size (theme::textSizes) - 1;
        view.addItem (item ("Larger Text",  KeyPress ('=', ModifierKeys::commandModifier, 0), theme::textSizes[jmin (last, index + 1)], index < last));
        view.addItem (item ("Smaller Text", KeyPress ('-', ModifierKeys::commandModifier, 0), theme::textSizes[jmax (0, index - 1)], index > 0));
        view.addItem (item ("Actual Size",  KeyPress ('0', ModifierKeys::commandModifier, 0), 100, textSize != 100));
        return view;
    }

    bool MorpherEditor::keyPressed (const KeyPress& key)
    {
        const auto cmd = key.getModifiers().isCommandDown();
        const auto c = key.getTextCharacter();
        const auto code = key.getKeyCode();

        int index = 0;
        for (int i = 0; i < (int) std::size (theme::textSizes); ++i)
            if (theme::textSizes[i] == textSize)
                index = i;

        const auto last = (int) std::size (theme::textSizes) - 1;

        if (cmd && (c == '=' || c == '+' || code == '='))  { setTextSize (theme::textSizes[jmin (last, index + 1)]); return true; }
        if (cmd && (c == '-' || code == '-'))              { setTextSize (theme::textSizes[jmax (0, index - 1)]);    return true; }
        if (cmd && (c == '0' || code == '0'))              { setTextSize (100); return true; }

        if (code == KeyPress::escapeKey)
        {
            // Esc lets go of the keyboard first; it never closes anything.
            if (auto* focused = Component::getCurrentlyFocusedComponent(); focused != nullptr && focused != this && isParentOf (focused))
            {
                grabKeyboardFocus();
                return true;
            }
        }

        return false;
    }
}
