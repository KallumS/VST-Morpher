# Review against Apple's Human Interface Guidelines: Foundations

Morpher checked against all 19 Foundations pages, item by item, the same
review the family's windows had (the guideline column is copied from Good
Idea's review, October 2026). The decision is
[0014](decisions/0014-apples-hig-foundations.md).

Morpher's window is drawn by JUCE inside a DAW. That reaches further than a
ReaImGui script: screen readers can read it. But a plug-in still can't read
Dynamic Type, Increase Contrast or the system's appearance, and keys reach it
only if the DAW passes them on. Where a guideline isn't met for that reason,
this says so rather than counting it as met.

## In brief

| Guideline | Here |
| --- | --- |
| **Typography** | The system's sans-serif at 13 points, regular weight, everywhere: buttons, menus, tooltips, messages. |
| **Larger text** | **View → Text Size**: 100%, 125%, 150% or 200%, also Cmd+= and Cmd+- (Ctrl on Windows), Cmd+0 for 100%. Remembered. The whole window grows with it. |
| **Control size and spacing** | Buttons 28 points tall, 12 apart, at least as wide as their label plus 12 each side. |
| **Contrast** | Every line of text 7:1 or more; edges you grab 3:1 or more (see [COLOUR.md](COLOUR.md)). |
| **More than colour alone** | A chosen button also has an ink bar under its label; every corner's share is a number as well as a bar; warnings are worded. |
| **The keyboard alone** | Tab moves between controls, Space or Return presses one, a near-white ring shows where you are; arrow keys move the pad (Shift for small steps); Esc lets go. |
| **Screen readers** | The window, each corner ("Corner A: Pro-Q 3, level 46%"), each button and the pad, as two sliders, have names. New messages are announced. |
| **Layout** | The message at the top; each corner's panel by its corner; buttons wrap rather than run off; nothing overlaps at any size. |
| **Avoid time-outs** | A message stays until **Dismiss**, or a newer one replaces it. |
| **Mistakes cheap** | **Undo** after Remove or Replace; a plug-in that can't be found is kept with its settings. |
| **Motion** | Nothing animates. The handle follows the pointer; the sound follows it over 20 ms so it doesn't zip. |
| **Privacy** | Nothing is collected or sent. |

## Already met

- **Design principles.** One job: mix four plug-ins by where you are on a
  square. Every corner works with nothing in it (it passes the sound
  through), so there's nothing to set up before you hear something, and a
  new Morpher changes nothing. Undo covers the two actions that lose
  settings; moving the pad is a host parameter change, which most DAWs can
  undo.
- **Feedback.** Buttons change on hover and press; the handle turns yellow
  while held; every corner's share updates as you move, in the pad and in its
  panel.
- **Colour.** The house scheme, the accent only for what is on
  ([0009](decisions/0009-the-handle-takes-the-accent-only-while-held.md)).
- **Dark Mode.** Always dark: allowed for media apps, and the house scheme.
- **Writing.** Plain words, verbs first, title style on controls, "plug-in" as
  Apple spells it, no "we", errors that say how to fix them.

## Not met, because a plug-in can't

- **Dynamic Type, Increase Contrast, Reduce Transparency** can't be read from
  inside a plug-in. Text Size and the 7:1 colours stand in.
- **Keys** reach a plug-in's window only if the DAW passes them. Many DAWs
  keep Space for play and stop.
- **A hosted plug-in's own window** is drawn by that plug-in; Morpher can't
  make it meet these guidelines.
- **Right to left.** English only.

## Not relevant

Materials (Liquid Glass), app icons, icons and SF Symbols, images, spatial
layout and immersive experiences: the window has none of these.

## One short of the family's bar

- **A held button's ink is 6.0:1** (`#14171C` on `#8F96A2`), above Apple's
  4.5:1 minimum for 13-point text but under the 7:1 every other pair meets.
  It lasts only while the button is pressed. Lightening the held grey would
  change the house scheme, so it is left for you to decide.

## Unverified in a DAW

Morpher was built and its tests run on Linux, without a screen, here; the
Mac and Windows builds run the same tests and pluginval in CI. Only a DAW can
confirm:

- that VoiceOver and Narrator read the names as intended;
- that Tab, the arrow keys, Esc and Cmd+=, Cmd+- and Cmd+0 reach the window
  in each DAW (the View menu works either way);
- that a hosted plug-in's window stays in front of the DAW's as intended;
- how the colours look on a real screen.

## Checked by

`tests/ThemeTests.cpp`: every text colour 7:1 on every surface it sits on;
every button's ink 7:1 at rest, hovered and chosen; edges, the handle, the
focus ring and the scrollbar 3:1; menus lighter than the window; the focus
ring and warnings not the accent's hue; R < G < B in every grey; the house's
four values; 13-point regular type; 28-point controls 12 apart; tooltips at
64 characters.

`tests/EditorTests.cpp`: in four states (empty; two loaded; four loaded with
a message; one missing with a message) at every text size and four window
sizes from the smallest up: every button 28 tall, at least square, wide
enough for its label, reachable by Tab, named and with a tooltip; buttons 12
apart and never overlapping; nothing off the window's edge; the pad, panels
and message never overlapping; panels tall enough for their buttons; the
message above everything; each panel by its corner. Also: Text Size and its
keys, remembered and snapped to an offered size; the pad's arrow keys and Esc;
the pad as two named sliders for screen readers; Edit's window opening, shown
as on, and closing before its plug-in goes.

`tests/ProcessorTests.cpp` loads a real VST3 (built with the tests) into the
corners: each corner alone at its corner and mixed between; empty corners
passing the sound through; one click in, one click out with a 100-sample
plug-in in one corner; the host told the delay; bypass keeping time; long
host blocks; mono; saving and reopening; a missing plug-in kept and saved
unchanged; Undo after Remove and Replace; a bad file refused in plain words.

Each of these was broken on purpose and the suite failed: no latency
compensation; a corner's delay never applied; buttons 8 apart; the message
moved to the bottom; corner A's weight wrong; buttons that never wrap.

## Every item

| Item | Guideline (paraphrased) | Verdict | Here |
| --- | --- | --- | --- |
| ACC-01 | Interactions should be familiar and behave the same everywhere | met | The house's buttons, words and keys |
| ACC-02 | Never carry information by one sense or channel only | met | A corner's share is a number and a bar; an open window's Edit has an ink bar; warnings are worded |
| ACC-03 | Adapt to people: honour their system settings or let them personalise | met | Text Size in the window; a plug-in can't read the system's settings |
| ACC-06 | Let text be enlarged to at least 200% | met | Text Size 100–200% ([0008](decisions/0008-text-size-scales-the-whole-window.md)) |
| ACC-07 | Let icons and glyphs grow too, not only words | met | The whole window scales: the handle, bars and letters with the words |
| ACC-08 | macOS type: 13 pt by default, never below 10 pt | met | 13 pt throughout; nothing smaller |
| ACC-09 | Thin weights need bigger sizes | met | Regular weight only |
| ACC-10 | Minimum contrast: 4.5:1 for text up to 17 pt; 3:1 at 18 pt and above or in bold | met | Every word 7:1 |
| ACC-11 | Contrast also applies to icons, not only text | met | The pad's edge 4.7:1, the handle 8.4:1, the scrollbar thumb 3.6:1, the resize corner 4.7:1 |
| ACC-13 | Check contrast in every appearance you ship | met | One palette |
| ACC-15 | Use shape, icon or text as well as colour | met | Chosen = colour + bar; shares are numbers; a missing plug-in is worded |
| ACC-16 | Watch the colour-blind pairings (red–green, blue–orange) | met | No two states differ by hue alone |
| ACC-17 | Consider letting people recolour charts and characters | user's decision | The house colours are settled |
| ACC-18b | (derived) VoiceOver can at least read a window's title | met, unverified | A JUCE plug-in reaches VoiceOver and Windows' screen readers: the window, each corner, each button and the pad (as two sliders, Morph X and Morph Y) have names; not yet tried with VoiceOver itself |
| ACC-18c | Native dialogs reach VoiceOver | met | Choose File… is the system's own file dialog |
| ACC-21 | Pair audio cues with visual cues | not applicable | Morpher makes no sounds of its own |
| ACC-22 | macOS control size: 28×28 pt by default, 20×20 pt at the least | met | Every button 28 pt tall and at least 28 wide (tested) |
| ACC-23 | About 12 pt of space around controls with a bezel | met | 12 pt apart (tested) |
| ACC-24 | About 24 pt of space around clickable things with no bezel | met | The pad, the one frameless target, is 100 pt tall or more and the window's width |
| ACC-25 | Use the simplest gesture for frequent actions | met | Single clicks and one drag |
| ACC-26 | Every gesture has an on-screen alternative | met | The pad from the keyboard: arrow keys, Shift for small steps; the host can automate it |
| ACC-28 | Siri and Shortcuts for voice and automation | met | Morph X and Morph Y are host parameters, so a DAW's automation and controller mapping reach them |
| ACC-29 | Work with Full Keyboard Access, Switch Control and Pointer Control | met, unverified | Tab and the arrow keys, if the DAW passes keys to the plug-in |
| ACC-30 | Everything works from the keyboard alone | met, unverified | Every control is reachable by Tab, if the DAW passes keys; a hosted plug-in's own window is its own |
| ACC-31 | Don't override system shortcuts | met | Only Cmd+=, Cmd+- and Cmd+0 |
| ACC-32 | Keep actions simple; prefer familiar system behaviour over custom | met | Esc lets go of the keyboard's focus; it never closes anything |
| ACC-33 | Avoid UI that times out; dismiss explicitly | met | Messages stay until Dismiss ([0011](decisions/0011-messages-at-the-top-and-undo.md)) |
| ACC-34 | Offer difficulty accommodations in games | not applicable | Not a game |
| ACC-35 | Never autoplay sound without visible start and stop controls; offer an opt-out | not applicable | Nothing plays by itself: Morpher changes the sound passing through |
| ACC-37 | Be careful with fast or blinking animation; under Reduce Motion, cut automatic or repeating zoom, scale and peripheral motion | met | Nothing animates; the handle follows the pointer |
| ACC-38 | Reduce Motion: stiffen springs, no bounce | met | Nothing animates; the handle follows the pointer |
| ACC-39 | Reduce Motion: motion follows the pointer directly | met | Nothing animates; the handle follows the pointer |
| ACC-40 | Reduce Motion: no animated depth changes | met | Nothing animates; the handle follows the pointer |
| ACC-41 | Reduce Motion: fade instead of sliding | met | Nothing animates; the handle follows the pointer |
| ACC-42 | Reduce Motion: don't animate into or out of blur | met | Nothing animates; the handle follows the pointer |
| ACC-43 | Confirm before hard-to-undo actions | met | Remove and Replace… leave an Undo; loading into an empty corner is undone by Remove |
| APP-14 | No replicas of Apple hardware in any artwork | not applicable | No artwork |
| BRA-01 | One voice and tone in every piece of copy | met | House words |
| BRA-02 | Use the accent colour sparingly: the primary action, status and the selected item, not every control | met | Yellow only for on: a held handle, an open window's Edit |
| BRA-03 | Brand colour can live in the content rather than on controls | met | No brand colour |
| BRA-04 | A custom font must be legible at every size and scale with the text setting; use the system font for body text and captions | met | System sans |
| BRA-05 | Express the brand through familiar components; keep standard size, placement and behaviour | met | Standard buttons, menu bar, menus and tooltips |
| BRA-06 | Branding gives way to content: no space spent only on a logo | met | No logo |
| BRA-07 | Standard patterns: controls in expected places, standard symbols, standard navigation and modality | met | A View menu; buttons with their usual meanings |
| BRA-08 | Don't repeat the logo through the app | met | No logo |
| BRA-09 | No branded launch or splash screen; a welcome screen is fine | met | Opens on the last state |
| BRA-10 | Keep Apple trademarks out of names and images | met | No Apple names; Steinberg's "VST" kept out of the name too ([0012](decisions/0012-named-morpher.md)) |
| COL-01 | One colour, one meaning; never style static text in the "interactive" colour | met | The accent is never on words |
| COL-02 | Colours work in light, dark and increased contrast | met | Dark only; a plug-in can't read the system's appearance |
| COL-04 | Test under bright and dim lighting | not possible | Only on a real screen; for the user |
| COL-05 | Test on several displays and colour profiles (sRGB vs P3, True Tone) | not possible | Only on a real screen; for the user |
| COL-06 | Artwork and translucency change how nearby colours look; adjust the neighbours | met | Only the corners' letters and shares sit on the pad, 7:1 or more |
| COL-07 | If people pick colours, use the system colour picker | user's decision | No colour choice (settled) |
| COL-08 | Never rely on colour alone to tell objects or states apart, show interactivity or carry essential information | met | See ACC-15 |
| COL-09 | Avoid hard-to-see combinations | met | The handle's grey and held yellow differ in lightness too |
| COL-10 | Colours mean different things in different cultures | met | Warnings are worded |
| COL-12 | Each palette role only for its job | met | #8A919C only for edges, never for words |
| COL-14 | Emphasise a primary action with a filled background, not coloured label text; don't fill many controls with the accent | met | No accent-filled primary button; accent = on |
| COL-15 | Over colourful content, keep control labels monochrome, or pick an accent clearly unlike the content | met | Labels monochrome |
| COL-16 | Keep content colours from clashing with control colours where they meet; legible at rest | met | The pad on its own ground with an edge |
| COL-17 | Embed a colour profile in images (sRGB is fine) | not applicable | No images |
| COL-21 | Alternate row backgrounds | not applicable | No lists or tables |
| COL-22 | Show selection more softly when the window isn't key | user's decision | As the family: dimming when the DAW has focus would make a tool window look off while you work |
| COL-24 | (tvOS) Don't show focus by colour alone | met | The focus ring is a shape |
| COL-25 | (visionOS) Colour works best in bold text and large areas | met | Accent on the handle and a button's fill only |
| COL-26 | (visionOS) Balance brightness; no bright flashing object on a black ground | met | No flashes |
| COL-27 | (watchOS) Colour a background only when it says something; avoid full-window colour on long-lived views | met | No coloured backgrounds |
| DRK-04 | A dark-only interface is fine in rare, media-focused cases | met | Dark only: house scheme, studio use |
| DRK-05 | A dark palette is not an inversion: dim the backgrounds, brighten the foregrounds | met | The house ramp |
| DRK-07 | Contrast 4.5:1 at minimum, aiming for 7:1 for small text | met | 7:1 |
| DRK-08 | Soften white backgrounds in images shown in a dark context | not applicable | No white page |
| DRK-10 | Give an edge to icons that would vanish on a dark ground | met | No dark glyphs on dark |
| DRK-11 | Full-colour images must read in your appearance; adjust them if not | not applicable | No pictures |
| DRK-12 | Use the label tiers (primary to quaternary) | met | Body, secondary and menu tiers |
| DRK-13 | Prefer system text fields | not applicable | No text fields; Choose File… is the system's dialog |
| DRK-14 | Raised surfaces lighter | met | Menus and tooltips #2A2F37 with a #3A404A edge, lighter than the window |
| DP-01 | Every feature serves the app's purpose | met | See "Already met" |
| DP-02 | Focus: make the core features excellent | met | See "Already met" |
| DP-03 | Don't rebuild what already exists; find your own angle | met | See "Already met" |
| DP-04 | Stay out of the way: take people straight to the task | met | See "Already met" |
| DP-05 | Freedom to explore; no lock-in modes; guided flows can be skipped or escaped | met | Nothing is modal; a menu closes with Esc |
| DP-06 | Make mistakes cheap to recover from | met | Undo for Remove and Replace; a missing plug-in is kept ([0010](decisions/0010-a-missing-plug-in-is-kept.md)); most DAWs can undo a move of the pad |
| DP-07 | Be transparent about background behaviour | met | Morpher's delay is reported to the host; an empty corner says it passes the sound through |
| DP-08 | Collect only what's needed and keep it safe | met | See "Already met" |
| DP-09 | Build on concepts people know (the real world and other software) | met | See "Already met" |
| DP-10 | Once a look or behaviour is set, apply it everywhere | met | See "Already met" |
| DP-11 | Clear feedback: show what's available, show when content changes, use system patterns for alerts and choices | met | See "Already met" |
| DP-12 | Design for everyone; accessibility from the start | met | See "Already met" |
| DP-13 | Preserve context | met | Each panel sits by its corner; the pad shows every corner's share |
| DP-14 | Support many input methods | met | See "Already met" |
| DP-15 | Equal care on every platform supported | not possible | Built and tested on Linux here; the Mac and Windows builds run the same tests in CI, but it hasn't yet been used in a DAW on either |
| DP-16 | Include only what's needed; let secondary items recede | met | See "Already met" |
| DP-17 | Be concise in labels and copy | met | See "Already met" |
| DP-18 | Clear hierarchy: people know where they are and what's next | met | See "Already met" |
| DP-19 | Craft: deliberate visuals, smooth motion, precise words, thoughtful sound | met | See "Already met" |
| DP-20 | Prototype, iterate, and test in real settings | not possible | Needs a DAW; see below |
| DP-21 | Keep maintaining after release | met | See "Already met" |
| DP-22 | Decide the emotion to evoke | met | See "Already met" |
| DP-23 | Treat small moments, error messages included, as chances for character | met | See "Already met" |
| DP-24 | Delight is not decoration; never at the task's expense | met | See "Already met" |
| ICN-01 | Simple, recognisable glyphs, using a metaphor tied directly to the action or content | not applicable | No icons: every control is a word |
| ICN-02 | All interface icons share size, detail, stroke weight and perspective | not applicable | No icons: every control is a word |
| ICN-03 | Adjust dimensions optically so light and heavy shapes look equal | not applicable | No icons: every control is a word |
| ICN-04 | Match an icon's weight to the text beside it | not applicable | No icons: every control is a word |
| ICN-05 | Centre asymmetric glyphs optically, not geometrically (pad them) | not applicable | No icons: every control is a word |
| ICN-06 | Make a separate selected-state icon only when needed | not applicable | No icons: every control is a word |
| ICN-07 | Inclusive imagery: gender-neutral figures, nothing culture-bound | not applicable | No icons: every control is a word |
| ICN-08 | Letters in an icon only when essential, and localised | not applicable | No icons: every control is a word |
| ICN-09 | Vector icons, or bitmaps at every scale | not applicable | No icons: every control is a word |
| ICN-10 | Text names for icon-only controls | not applicable | No icon-only controls: every control is a word |
| ICN-11 | No replicas of Apple hardware | not applicable | No icons: every control is a word |
| ICN-12 | Use the standard meanings: scissors = cut, two pages = copy, clipboard = paste, check = done or save, X = cancel or close, trash = delete, curved arrows = undo and redo, + = add, … = more, pencil = rename, folder, magnifier = search, three lines = filter, box with up arrow = share or export, printer = print | not applicable | No icons: every control is a word |
| IMG-01 | Lay out in points; the scale factor maps points to pixels | met | Laid out in points; JUCE maps points to pixels |
| IMG-02 | macOS needs @1x and @2x bitmaps | not applicable | No images |
| IMG-03 | Design at 1×; put vector points and edges on whole points so they stay crisp at 2× | not applicable | No images |
| IMG-04 | Formats: de-interlaced PNG; an 8-bit palette when full colour isn't needed; JPEG or HEIC for photos; PDF or SVG for flat art | not applicable | No images |
| IMG-05 | Embed a colour profile | not applicable | No images |
| IMG-06 | Test images on real screens | not applicable | No images |
| IMM-02 | Offer several ways to use it and respect accessibility settings | not applicable | No immersive stages |
| IMM-03 | Start in the least immersive mode; let people opt in | not applicable | No immersive stages |
| IMM-04 | Save immersion for meaningful moments | not applicable | No immersive stages |
| IMM-05 | Guide attention with gentle cues first, and stronger ones only if needed | not applicable | No immersive stages |
| IMM-09 | Don't require people to move | not applicable | No immersive stages |
| IMM-12 | Smooth, predictable transitions | not applicable | No immersive stages |
| IMM-13 | Let people choose when to enter and leave; a clear exit | not applicable | No immersive stages |
| IMM-14 | The exit says whether it goes back or quits; offer to save before quitting | not applicable | No immersive stages |
| IMM-16 | Environments: few distractions, subtle animation, nothing moving at the edges | not applicable | No immersive stages |
| IMM-17 | Mark interactive objects clearly | not applicable | No immersive stages |
| IMM-18 | Preview doesn't stack on playback unexpectedly | not applicable | Morpher never plays by itself |
| INC-01 | Learn people's goals and perspectives; keep revising | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-02 | Weigh age, gender, culture, ability, education, economics and more | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-03 | Inoffensive isn't the same as inclusive | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-04 | Plain, welcoming tone, not academic | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-05 | Address people as "you"; avoid "the user" or "the player"; keep "we" for the maker | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-06 | Define specialised terms, or say it plainly; make definitions easy to find | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-07 | Swap idioms and colloquialisms for plain words | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-08 | Be careful with humour (it translates badly and wears thin) | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-09 | Approachable: a clear interface, with a way to learn that experts can skip | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-10 | Drop needless gender references ("his or her") | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-11 | Avatars, glyphs and characters not gendered, or customisable | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-12 | Generic people drawn as non-gendered figures | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-14 | Show varied people; avoid role stereotypes | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-15 | Relatable settings and objects; don't assume wealth | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-16 | Question built-in assumptions (family, life events) | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-17 | Support accessibility features; never assume disability means no interest | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-18 | Disability is a spectrum, and also temporary or situational | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-19 | No disability used as a negative ("blind to", "lame", "crippled", "dumb", "sanity check") | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-20 | People-first wording about disability | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-21 | Simple and perceivable | met | Plain musical words; no gendered or ableist words (grepped) |
| INC-22 | Prepare for other languages and regions (date, time, number formats) | not applicable | English only |
| INC-24 | Check what colours mean in each locale you localise for | not applicable | English only |
| LAY-01 | Most important items at the top and leading edge | met | The message at the top, then A and B |
| LAY-02 | Align for scanning; indent to show subordination | met | Each panel: name, what it is, its share, its buttons |
| LAY-03 | Group related items with space, containers or separators | met | Each corner in its own panel |
| LAY-04 | Progressive disclosure | met | A corner's buttons appear only when they'd do something |
| LAY-05 | Make controls look distinct from content | met | The pad on its own dark ground, apart from the controls |
| LAY-07 | Adapt to window sizes, aspects and displays; respect margins | met | Tested from the smallest window, 480×560, to 1400×900, at 100% to 200% |
| LAY-08 | Text growth: wrap instead of crop | met | Buttons and messages wrap |
| LAY-09 | Test the largest and smallest layouts | met | Tested: empty, loaded, missing, with and without a message, every size |
| LAY-10 | Scale background art to fill without changing its aspect ratio | not applicable | No art |
| LAY-11 | (iOS) Lay out by available space (size classes), not device | met | Below 980 pt the panels go above and below the pad |
| LAY-12 | Same functions at every size; extras can move into an overflow | met | Same controls at every size |
| LAY-13 | The layout stays recognisable after a resize | met | Same order at any size |
| LAY-14 | Limit line length | met | Tooltips broken at 64 characters |
| LAY-15 | Respect safe areas | not applicable | Hosted window |
| LAY-16 | (macOS) No controls or critical information at the window's bottom edge | partly | The message is at the top; C's and D's buttons sit by their corners, 16 pt above the bottom edge ([0013](decisions/0013-panels-beside-their-corners.md)) |
| LAY-19 | (tvOS) Even spacing in grids | met | One gap |
| LAY-20 | (visionOS) Allow resizing; a min and max size stop overlap, but don't use them to block resizing | met | Resizable; smallest 480×560, grown with the text |
| LAY-21 | (visionOS) Centre content in very large windows | met | The pad grows to fill |
| MAT-02 | Over bright content, put about a 35% dark dimming layer behind clear-glass controls | met | No text over graphics |
| MAT-03 | Pick materials by meaning, not by the tint they give | met | Colours by role |
| MAT-05 | Thicker (more opaque) backings for fine text; thinner ones keep context | met | No translucent fills |
| MAT-09 | (derived) Reduce Transparency equivalent | met | Solid window |
| MOT-01 | Only purposeful motion | met | Nothing moves by itself; the handle follows the pointer |
| MOT-02 | Motion is optional and never the only carrier of information | met | Nothing moves by itself; the handle follows the pointer |
| MOT-03 | Feedback motion follows the gesture and expectation | met | Nothing moves by itself; the handle follows the pointer |
| MOT-04 | Feedback animation is short and precise | met | Nothing moves by itself; the handle follows the pointer |
| MOT-05 | No added motion on frequent interactions | met | Nothing moves by itself; the handle follows the pointer |
| MOT-06 | Motion can be cancelled; never make people wait for it | met | Nothing moves by itself; the handle follows the pointer |
| MOT-08 | Games: a steady 30–60 fps by default | not applicable | Not a game |
| MOT-09 | Let people trade visuals for performance or battery | met | Nothing moves by itself; the handle follows the pointer |
| MOT-10 | (visionOS) No motion in the periphery; large moving objects; no rotating world | met | Nothing moves by itself; the handle follows the pointer |
| MOT-11 | (visionOS) Fade, don't fly, when relocating | met | Nothing moves by itself; the handle follows the pointer |
| MOT-12 | (visionOS) A stationary frame of reference | met | Nothing moves by itself; the handle follows the pointer |
| MOT-13 | (visionOS) Avoid sustained oscillation, worst near 0.2 Hz (a 5-second cycle); keep the amplitude low | met | Nothing moves by itself; the handle follows the pointer |
| MOT-15 | (derived, ACC) No flashing above 3 per second | met | Nothing moves by itself; the handle follows the pointer |
| PRV-01 | Ask only for what you need | met | Nothing collected or sent; the text size and recent plug-ins in a settings file on this computer, the rest in the project |
| PRV-02 | Be transparent about what's collected and how it's used | met | Nothing collected or sent; the text size and recent plug-ins in a settings file on this computer, the rest in the project |
| PRV-03 | Process on the device | met | Nothing collected or sent; the text size and recent plug-ins in a settings file on this computer, the rest in the project |
| PRV-04 | Use system protections and secure practices | met | Nothing collected or sent; the text size and recent plug-ins in a settings file on this computer, the rest in the project |
| PRV-06 | Ask at the moment of use, not at launch | met | Nothing collected or sent; the text size and recent plug-ins in a settings file on this computer, the rest in the project |
| PRV-13 | Never keep passwords or secrets in plain-text files | met | Nothing collected or sent; the text size and recent plug-ins in a settings file on this computer, the rest in the project |
| PRV-16 | (macOS) Don't assume who is signed in (fast user switching) | met | Nothing collected or sent; the text size and recent plug-ins in a settings file on this computer, the rest in the project |
| RTL-04 | Every item in a list uses the same alignment | met | One alignment |
| RTL-10 | Never flip a control that means a real direction | not applicable | English only |
| SFS-03 | Licence: no SF Symbols, or look-alikes, in app icons, logos or trademarks | not applicable | No icons |
| SFS-04 | Rendering modes (monochrome, hierarchical, palette, multicolour) | not applicable | No icons |
| SFS-05 | Check icon legibility at each size and background | not applicable | No icons |
| SFS-07 | Variable colour shows a changing level (0–100%), not depth | not applicable | No icons |
| SFS-08 | Match symbol weight to the text weight | not applicable | No icons |
| SFS-09 | Size icons against the text's cap height (small, medium, large) | not applicable | No icons |
| SFS-10 | Variant conventions: slash = unavailable or off; fill = selected; enclosed = clearer at small sizes; outline next to text | not applicable | No icons |
| SFS-12 | Animate symbols sparingly, with purpose, in keeping with the app's tone | not applicable | No icons |
| SFS-13 | Animation meanings: bounce = something happened; pulse or breathe = ongoing; rotate = working; variable colour = progress; wiggle = look here | not applicable | No icons |
| SFS-14 | Custom symbols match the system set: simple, recognisable, inclusive, directly related | not applicable | No icons |
| SFS-15 | Negative side margins so badged icons align optically | not applicable | No icons |
| SFS-19 | No Apple product replicas | not applicable | No icons |
| SPA-03 | Use depth for hierarchy, on big elements, only where it adds value | met | No depth |
| SPA-04 | Don't give text depth | met | No text shadows |
| SPA-05 | Don't open too many windows | met | One window; a plug-in's own opens only on Edit, and closes with Morpher's |
| SPA-07 | Leave room round interactive elements (≥16 pt gaps there); never overlap them | met | No overlapping targets (tested) |
| SPA-08 | Usable with little physical movement | met | Panels by their corners |
| TYP-01 | macOS: 13 pt default, 10 pt minimum | met | 13 pt |
| TYP-02 | Test legibility in context; fix with size, contrast or a more legible face | not possible | Needs a real screen |
| TYP-03 | Avoid Ultralight, Thin and Light; use Regular through Bold | met | Regular |
| TYP-04 | Hierarchy through weight, size and colour, kept when text is scaled | met | One size, scaled as a whole |
| TYP-05 | Few typefaces | met | One face |
| TYP-06 | When scaling, the important content grows; minor chrome may not | met | Everything grows |
| TYP-07 | Use the system font (SF Pro on macOS) | met | JUCE's system sans (San Francisco on a Mac, Segoe UI on Windows) |
| TYP-08 | Don't embed or bundle the system fonts | met | No fonts shipped |
| TYP-09 | Use the macOS text styles: Large Title 26/32, Title 1 22/26, Title 2 17/22, Title 3 15/20, Headline 13 Bold/16, Body 13/16, Callout 12/15, Subheadline 11/14, Footnote 10/13, Caption 1 10/13, Caption 2 10 Medium/13 | met | Body 13 throughout |
| TYP-10 | Emphasise with the bold trait (Body emphasised = Semibold; Headline = Heavy) | user's decision | No bold headings, as in the family |
| TYP-11 | Looser leading for long or wide passages; tight leading only up to 2 lines | not applicable | No long passages; messages run to two or three lines |
| TYP-12 | Size-specific tracking (e.g. 10 pt +0.12 pt, 13 pt −0.08 pt, 17 pt −0.43 pt, 26 pt +0.22 pt) | user's decision | Not done: JUCE could set tracking, the family's windows don't |
| TYP-13 | Custom fonts stay legible; follow the minimum sizes | not applicable | No custom font |
| TYP-14 | Custom fonts honour accessibility (Bold Text, size changes) | met | Text Size |
| TYP-16 | The layout adapts to every text size | met | Buttons and messages wrap at every size |
| TYP-17 | Meaningful icons grow with the text | met | See ACC-07 |
| TYP-18 | Keep truncation to a minimum at large sizes; never truncate scrolling text without a way to read it all | partly | A very long plug-in name ends in … in its panel; screen readers get it whole, and its Edit window's title shows it |
| TYP-19 | Stack, or use fewer columns, at large sizes | met | Panels stack when narrow |
| TYP-20 | Primary elements stay at the top at any size | met | The message stays on top |
| TYP-21 | macOS control, menu and tooltip font variants match system controls | met | Labels, menus and tooltips 13 pt |
| WRI-01 | Choose a voice; keep a list of terms; use them consistently | met | House words: Choose, Replace, Edit, Remove, Undo, Dismiss; "plug-in" as Apple spells it |
| WRI-02 | Fit the tone to the moment (plain for problems, lighter for success) | met |  |
| WRI-03 | Be clear; cut words; read it aloud | met |  |
| WRI-04 | Write for everyone: plain, accessible, translatable, without jargon or gendered terms | met |  |
| WRI-05 | Most important first; easy to read; split separate ideas | met |  |
| WRI-06 | Active voice; buttons lead with a verb; clear rather than cute | met | Verbs first: Choose Plug-in…, Replace…, Edit, Remove, Undo, Dismiss |
| WRI-07 | Descriptive link text, never "Click here" | not applicable | No links |
| WRI-08 | Reuse language patterns | met |  |
| WRI-09 | One capitalisation style per kind of element, used throughout | met | Title style on controls |
| WRI-10 | Multi-step flows: consistent Get Started → Continue/Next → Done | not applicable | No multi-step flows |
| WRI-11 | Few possessives ("Favorites", not "Your Favorites"); one perspective | met |  |
| WRI-12 | Avoid "we" (errors read "Unable to …") | met | No "we" (grepped) |
| WRI-13 | The right verbs for the device ("click" on a Mac) | met | No device verbs needed |
| WRI-14 | Empty states say what to do | met | An empty corner says what it does: "Passes the sound through" |
| WRI-15 | Nothing crucial only in a temporary state | met | A missing plug-in is marked in its panel as well as the message |
| WRI-16 | Prevent errors first | met | No dimmed controls: buttons appear when they'd do something |
| WRI-17 | Show an error as close to the problem as possible | met | Messages at the top (family decision); a missing plug-in is also marked in its own panel |
| WRI-18 | No blame; say how to fix it, positively | met | "Couldn't open X. Check that it's a 64-bit VST3 plug-in and that it opens on its own in your DAW." |
| WRI-19 | No "Oops" or "Uh-oh" | met |  |
| WRI-20 | If many people hit an error, redesign the interaction | met |  |
| WRI-21 | Choose how to deliver a message by its urgency | met | Message line, no dialogs |
| WRI-22 | Toggle hints describe the on state | met | Edit's tooltip says what it shows |
| WRI-23 | Send people to a setting with a button, not directions | met | Text Size in the View menu |
| WRI-24 | Text fields: a clear label | not applicable | No text fields |
| WRI-25 | Field errors beside the field | not applicable | No text fields |
| APP-01 | A distinctive app icon | not applicable | A plug-in has no app icon |
| COL-11 | Don't hard-code system colours | met | Colours named by role in `src/Theme.h` |
| COL-20 | macOS's dynamic colour roles | met | A token for each role: text, secondary, warning, edge, separator, focus ring |
