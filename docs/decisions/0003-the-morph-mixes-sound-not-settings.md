# 0003. The morph mixes the sound, not the settings

## Context

"Morphing" can mean blending four plug-ins' sound, or blending one plug-in's
settings between four presets. The second only works with four of the same
plug-in, and not every setting blends sensibly.

## Decision

Every corner's plug-in processes the same input, and Morpher mixes their
outputs. Where you are on the pad sets each corner's share, bilinearly:

- A, top left: (1 − x) · y
- B, top right: x · y
- C, bottom left: (1 − x) · (1 − y)
- D, bottom right: x · (1 − y)

The shares always add up to 1, and are mixed at their own level, not equal
power. Four plug-ins fed one input usually sound alike enough that equal power
would swell by up to 6 dB in the middle.

Position is smoothed over 20 ms, sample by sample, so dragging and automation
don't zip.

## Consequences

- Any four plug-ins can be morphed, effects or instruments.
- Four plug-ins that cancel each other (opposite phase) will dip between
  corners; that is the sound, not a fault.
- The host can automate the pad as two parameters, Morph X and Morph Y.
