# 0004. An empty corner passes the sound through

## Context

With fewer than four plug-ins, something has to be heard at an empty corner.

## Decision

An empty corner, or one whose plug-in can't be found, passes the input
through unchanged (lined up in time, decision 0005). So does a corner whose
plug-in makes no sound of its own (a MIDI effect).

## Consequences

- One plug-in in A and nothing else is a dry/wet control running from A to the
  other corners.
- An empty corner is never silent, which is said under it: "Passes the sound
  through".
