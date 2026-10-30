# 0006. A new Morpher starts at corner A, with up at the top

## Context

A new instance has to start somewhere. In the middle, a plug-in loaded into A
is a quarter of the sound, which reads as the plug-in not working.

## Decision

The pad starts in corner A (Morph X 0%, Morph Y 100%). Y runs up, as an
automation lane does: 100% is the top.

## Consequences

- Load a plug-in into A and you hear all of it at once.
- Before anything is loaded, every corner passes the sound through, so a new
  Morpher changes nothing.
