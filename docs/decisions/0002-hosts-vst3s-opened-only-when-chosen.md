# 0002. Hosts VST3 plug-ins, opened only when you choose one

## Context

Hosts usually scan every installed plug-in, opening each one. Inside a DAW, a
plug-in that crashes while being scanned takes the DAW, and the project, with
it. Morpher runs inside a DAW.

## Decision

Morpher never scans. **Choose Plug-in…** offers:

- **Recent**: plug-ins chosen before, most recent first (twelve kept).
- **Installed**: the `.vst3` files in the usual VST3 folders, found by file
  name alone, grouped by maker's folder. Nothing is opened until you choose
  one.
- **Choose File…**: any `.vst3` anywhere.

It hosts VST3s only; not AU, CLAP or VST2.

## Consequences

- Only the plug-in you choose is ever opened.
- The Installed list shows file names, which are usually but not always the
  plug-in's name.
- A file holding several plug-ins (a "shell") asks which one.
- Hosting CLAPs would need a CLAP host written for JUCE; hosting AUs on a Mac
  is in JUCE and could be added, chosen from a list rather than a file.
