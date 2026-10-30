# 0013. Each corner's panel sits by its corner

## Context

The four panels need to read as belonging to the four corners.

## Decision

- Usually (narrower than 980 points): A and B above the pad, C and D below it,
  each on its corner's side.
- Wide: A and C to the left of the pad, B and D to the right, at the top and
  bottom.
- The pad shows only each corner's letter and share; names are in the panels.

## Consequences

- C's and D's buttons sit near the bottom of the window, by their corners.
  Apple advises against controls at a Mac window's bottom edge; here they keep
  16 points from it, and the message line is at the top.
