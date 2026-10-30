# 0008. Text Size scales the whole window

## Context

Apple asks that text can be made at least 200% larger, and that controls and
layout grow with it. A plug-in can't read the system's text size.

## Decision

**View → Text Size**: 100%, 125%, 150% or 200%, also Cmd+= and Cmd+- (Ctrl on
Windows and Linux), Cmd+0 for 100%. Shared by every Morpher and remembered.

Everything is laid out in points at 100% and the whole window is scaled, so
words, buttons, the pad and the window grow together. The window keeps its
size in points: at 200% it is twice as wide and tall.

## Consequences

- Nothing is cropped or crowded at any size: tested in every state at every
  size from the smallest window to a large one.
- If a 200% window is too big for the screen, make it smaller: the panels move
  above and below the pad, and their buttons wrap.
- Whether the DAW passes Cmd+= to a plug-in's window varies; the View menu
  does the same either way.
