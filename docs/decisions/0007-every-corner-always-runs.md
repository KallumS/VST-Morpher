# 0007. Every corner runs all the time

## Context

A corner at 0% could be skipped to save processor time.

## Decision

All four plug-ins process every block, heard or not.

## Consequences

- Moving toward a corner, its reverb tail, compressor and delay are already
  where they'd be: no jump, no gap.
- Morpher costs what its four plug-ins cost together.
