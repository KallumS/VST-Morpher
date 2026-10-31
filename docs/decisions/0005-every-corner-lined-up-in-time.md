# 0005. Every corner is lined up in time with the slowest

## Context

Plug-ins delay their sound by different amounts (their latency). Mixed as
they come, a delayed copy against an undelayed one combs and smears.

## Decision

Morpher holds each corner back by the difference between its plug-in's delay
and the slowest one's, empty corners included, and tells the host its own
delay is the slowest one's, so the host lines Morpher up with the other
tracks. When a plug-in changes its delay, Morpher follows.

Bypassed, Morpher delays the dry sound by the same amount, so it stays in time.

## Consequences

- A click in comes out as one click: tested with a real VST3 at a 100-sample
  delay.
- Up to 2 seconds can be lined up. A plug-in that delays more gets a warning
  saying the corners will be out of time.
- Morpher's delay is the slowest plug-in's even when that corner is silent.
