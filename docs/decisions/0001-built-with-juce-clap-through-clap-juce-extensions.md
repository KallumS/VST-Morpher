# 0001. Built with JUCE; CLAP through clap-juce-extensions

## Context

Morpher is a plug-in that hosts plug-ins. It has to be a VST3 and a CLAP, and
it has to open VST3s itself. JUCE (9.0.4) builds VST3s and AUs, and hosts
VST3s; it doesn't build CLAPs.

## Decision

Build with JUCE, fetched by CMake at a fixed tag. Make the CLAP with
[clap-juce-extensions](https://github.com/free-audio/clap-juce-extensions),
fetched at a fixed commit. On a Mac, the AU comes free and is built too.

## Consequences

- One code base gives VST3, CLAP and AU.
- JUCE is AGPLv3 or commercial; Morpher is GPLv3, which may be combined with
  AGPLv3 code. Selling a closed-source build would need a JUCE licence.
- If JUCE adds CLAP itself, clap-juce-extensions can go.
