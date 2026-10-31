# 0014. Apple's Human Interface Guidelines: Foundations

## Context

The family's windows were reviewed against all 19 Foundations pages. Morpher
should meet the same bar from the start.

## Decision

Follow them, as checked in [the review](../apple-hig-review.md): 13-point
system text, 28-point controls 12 apart, 7:1 text, edges of 3:1, Text Size to
200%, the keyboard alone, screen-reader names, messages that don't time out,
Undo, and the house scheme.

Unlike a ReaImGui script, a JUCE plug-in can reach VoiceOver and other screen
readers, so it does.

## Consequences

- `tests/ThemeTests.cpp` and `tests/EditorTests.cpp` check what can be
  measured, in every state, at every text size.
