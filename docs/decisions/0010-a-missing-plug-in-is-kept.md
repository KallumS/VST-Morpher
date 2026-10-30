# 0010. A plug-in that can't be found is kept

## Context

A project may open on a computer without one of its plug-ins, or after a
plug-in has moved.

## Decision

- First Morpher looks for the same file in the usual VST3 folders and their
  makers' folders, and opens it from there.
- Failing that, the corner says "Not found. Its settings are kept." and the
  message line names the plug-in. The corner passes the sound through, and its
  settings are saved again unchanged, so they come back when it's installed.
- Choosing another plug-in, or Remove, is how to let it go.

## Consequences

- Opening and saving a project on the wrong computer loses nothing.
