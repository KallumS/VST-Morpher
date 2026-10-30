# 0011. Messages at the top; Remove and Replace can be undone

## Context

Apple: nothing important at a window's bottom edge, nothing that times out,
and make mistakes cheap. Removing or replacing a plug-in loses its settings.

## Decision

- One message line, at the top. A message stays until **Dismiss**, or a newer
  one replaces it. It lives in the plug-in, not the window, so closing and
  opening the window keeps it.
- After **Remove** or **Replace…**, the message has an **Undo** that puts the
  plug-in back with its settings.
- Loading into an empty corner says nothing: Remove undoes it.

## Consequences

- Screen readers announce each new message.
