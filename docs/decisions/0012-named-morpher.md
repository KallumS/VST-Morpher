# 0012. Named Morpher

## Context

The repository is VST-Morpher. "VST" is Steinberg's trademark, and Morpher is
also a CLAP and an AU.

## Decision

The plug-in is called **Morpher**, by Kallum Shah: `com.kallumshah.morpher`,
manufacturer code `Ksha`, plug-in code `Mrph`.

## Consequences

- Renaming later is a change to `CMakeLists.txt`, but changes the codes hosts
  know it by, so projects saved with it would no longer find it. Rename before
  release if at all.
