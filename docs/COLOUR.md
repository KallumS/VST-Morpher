# The colour scheme

Every colour Morpher's window uses. It is the house scheme, the same as Starting
Blocks, Midi Suggester, Midi Variator, Midi Catalogue, ScaleView and Good Idea:
a dark cool-grey ground, a light grey for the controls raised off it, and one
yellow for whatever is switched on.

This file was copied from Good Idea's and is **not generated**: it is a
reference kept by hand. If you change a colour in `src/Theme.h`, change it here
too. `tests/ThemeTests.cpp` checks the values below and their contrast.

Morpher has no piano roll. Its one new surface is the pad, which takes the
roll's ground, and its one new control is the pad's handle, which follows the
slider grab: the controls' grey at rest, the accent while it is held
([decision 0009](decisions/0009-the-handle-takes-the-accent-only-while-held.md)).

## The four that carry it

Everything else is support. These four are the scheme.

| | value | what it is |
| --- | --- | --- |
| Accent | `#FFF200` | whatever is switched on |
| Controls | `#A9AFBA` | buttons and the pad's handle, raised off the ground |
| Ground | `#23272E` | the window behind everything |
| Ink | `#14171C` | the text on any button |

The accent goes on one thing only: the state that is on. In Morpher that is
the Edit button while its plug-in's window is open, and the pad's handle while
you hold it. Nothing decorative takes it, which is what makes it read at a
glance.

## The grey ramp

The house ramp, with what each grey does here. Greys the house uses that
Morpher doesn't are kept, marked "not used", so the ramp stays one ramp.

| | R | G | B | used for |
| --- | ---: | ---: | ---: | --- |
| `#111419` | 17 | 20 | 25 | the pad (the house piano-roll ground); a level bar's track |
| `#14171C` | 20 | 23 | 28 | the text on every button, the handle's rim |
| `#1A1D23` | 26 | 29 | 35 | the corner panels (sunken frames), scrollbar track |
| `#1B1F25` | 27 | 31 | 37 | not used (menus are lighter now: `#2A2F37`) |
| `#1E2228` | 30 | 34 | 40 | not used |
| `#22262D` | 34 | 38 | 45 | not used |
| `#23272E` | 35 | 39 | 46 | the window, the menu bar |
| `#2A2F37` | 42 | 47 | 55 | menus and tooltips, raised off the window |
| `#3A404A` | 58 | 64 | 74 | separators, the pad's grid, a corner's letter badge, menu highlight, menu and tooltip edges |
| `#585F6B` | 88 | 95 | 107 | not used |
| `#6D7581` | 109 | 117 | 129 | scrollbar thumb (3.6:1 on its track) |
| `#8A919C` | 138 | 145 | 156 | the pad's edge (4.7:1), a resize corner at rest; never words |
| `#8F96A2` | 143 | 150 | 162 | button held |
| `#A9AFBA` | 169 | 175 | 186 | buttons, the handle, the level bars |
| `#B0B6C0` | 176 | 182 | 192 | secondary text: 7.4:1 on the window |
| `#BFC5CE` | 191 | 197 | 206 | headings and keys in menus: 7.8:1 on a menu |
| `#C0C6CF` | 192 | 198 | 207 | button hover |
| `#DDE1E7` | 221 | 225 | 231 | body text |
| `#F2F4F7` | 242 | 244 | 247 | the keyboard's focus ring |

## One colour off the ramp

`#FF9E96` - a light red, for warnings: in the message line and on a corner
whose plug-in can't be found. It is the family's warning colour since the
Apple review (the house red `#D2483F` was 3.4:1 and is no longer used for
words). Deliberately not a shade of the accent, so a warning can never read as
a selection. It and the yellow are the only saturated colours in the scheme.

## Hover and held

| | rest | hover | held |
| --- | --- | --- | --- |
| Controls | `#A9AFBA` | `#C0C6CF` | `#8F96A2` |
| Accent | `#FFF200` | `#FFF42E` | `#D1C600` |

The house scripts compute the accent's two with `shade()`, 18% toward white
and black; Morpher stores the results, since JUCE's own blend rounds `#FFF42E`
to `#FFF42D`.

## Two rules worth carrying

**Keep the greys blue.** R < G < B holds in every grey. A neutral grey at the
same lightness looks correct in a diff and only reads as flat beside the
yellow. The tests check it.

**Light controls force dark text.** Both button fills sit far lighter than the
ground, so the body text colour vanishes on them. Every button takes the ink,
chosen or not.

## The values

```css
:root {
  --accent:         #FFF200;
  --accent-hover:   #FFF42E;
  --accent-held:    #D1C600;
  --control:        #A9AFBA;
  --control-hover:  #C0C6CF;
  --control-held:   #8F96A2;
  --ground:         #23272E;
  --sunken:         #1A1D23;
  --pad:            #111419;
  --raised:         #2A2F37;
  --rule:           #3A404A;
  --edge:           #8A919C;
  --scroll-grab:    #6D7581;
  --ink:            #14171C;
  --text:           #DDE1E7;
  --text-secondary: #B0B6C0;
  --text-on-raised: #BFC5CE;
  --warning:        #FF9E96;
  --focus:          #F2F4F7;
}
```

## Contrast, measured

Ratios are WCAG's, which Apple's guidelines use. Text targets 7:1, the figure
for small text in a dark window; things you see and grab, 3:1.

| | on | ratio |
| --- | --- | ---: |
| Body text `#DDE1E7` | the window | 11.4:1 |
| Secondary `#B0B6C0` | the window / a panel / the pad | 7.4 / 8.3 / 9.1:1 |
| Warning `#FF9E96` | the window | 7.6:1 |
| Body text | a menu or tooltip `#2A2F37` | 10.3:1 |
| Menu headings and keys `#BFC5CE` | a menu | 7.8:1 |
| Ink `#14171C` | a button, at rest / hover / chosen | 8.2 / 10.5 / 15.4:1 |
| Ink | a held button `#8F96A2` | 6.0:1 (see the review) |
| The pad's edge `#8A919C` | the window | 4.7:1 |
| The handle `#A9AFBA` | the pad | 8.4:1 |
| The focus ring `#F2F4F7` | the window | 13.6:1 |
