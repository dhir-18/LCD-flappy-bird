# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A Flappy Bird clone that runs on a 16x2 character LCD, built for an ELEGOO UNO
starter kit (Arduino Uno). Single file: `flappy_bird_lcd.ino`. No external
libraries beyond the Arduino-bundled `LiquidCrystal` and `EEPROM`.

## Hardware / build

- Build and upload with the Arduino IDE (or `arduino-cli compile --fqbn arduino:avr:uno` /
  `arduino-cli upload`), targeting an Arduino Uno.
- Wiring is documented in the header comment: LCD on pins 12/11/5/4/3/2 (RS, E, D4-D7),
  contrast pot on V0, joystick VRy on A0, joystick button on pin 7 (`INPUT_PULLUP`),
  active buzzer on pin 8. VRx is unused.
- There's no automated test suite — verification is by flashing the board and playing it.

## How the code is organized

**Global state** (top of file): `birdPos` (0-15, the bird's vertical position across
both LCD rows), `colState[16]` (one entry per LCD column: 0 empty, 1 pipe blocking the
top row, 2 pipe blocking the bottom row), `score`/`highScore`, and `gameOver`. The game
has no separate "game object" — it's all flat globals mutated each tick, typical for
an Arduino sketch.

**`createBirdChars()`** — the trick that makes vertical movement look smooth on a
character LCD. A 16x2 char display can normally only show 2 vertical positions per
column. This defines 8 custom characters (char codes 0-7), each lighting up one pixel
row (0-7) of an 8-pixel-tall cell. Combined with picking the top or bottom LCD row,
that gives 16 distinct vertical positions (`birdPos` 0-7 → custom char in row 0,
8-15 → custom char `birdPos - 8` in row 1). This is why `MAX_POS` is 15.

**`resetGame()` / `loadHighScore()` / `saveHighScore()`** — game state reset and
EEPROM persistence. High score is stored at EEPROM address 0 via `EEPROM.get/put`.
`loadHighScore()` clamps to 0-999 to guard against reading garbage from a
never-written EEPROM.

**`readJoystickVelocity()`** — reads the analog joystick (`A0`, center ~512), applies
a dead zone (`JOY_DEADZONE = 60`) so small drift doesn't move the bird, and maps the
deflection to a velocity of -2..+2 positions per tick (`JOY_MAX_SPEED`). In
`stepGame()`, a velocity of 0 (joystick centered) is overridden to +1, which is the
"gravity" — the bird drifts downward unless actively steered.

**`beep()` / `gameOverTone()` / `highScoreFanfare()`** — buzzer feedback using
`tone()`. `gameOverTone()` plays a two-note descending tone; `highScoreFanfare()`
plays a 4-note rising arpeggio (C5-E5-G5-C6) when a new high score is set.

**`drawGame()`** — renders one frame. Iterates all 16 columns; at `BIRD_COL` (column 1)
it draws the bird's custom character (top or bottom row per `birdPos`), and at every
other column it draws a solid block (char 255) in the top or bottom row if `colState`
says a pipe occupies it. `lcd.clear()` + full redraw every tick is simple and fast
enough at the LCD's low column count.

**`stepGame()`** — the core tick, called on a non-blocking timer (see `loop()`):
1. Scores a point if the pipe currently at column 0 is non-empty (i.e. a pipe just
   scrolled fully off-screen).
2. Scrolls `colState` left by one (`colState[i] = colState[i+1]`).
3. Rolls a new column at the right edge: 35% chance of a pipe, randomly top or bottom
   half.
4. Applies joystick velocity (or gravity) to `birdPos`, clamping to `[0, MAX_POS]`;
   hitting the bottom clamp (`MAX_POS`) is a game over ("hit the ground").
5. Checks collision: if `colState[BIRD_COL]` blocks the half of the LCD the bird is
   currently in, it's game over.
6. On game over, calls `checkHighScore()`.

**`checkHighScore()`** — updates and persists `highScore` if `score` beat it, and sets
`isNewHighScore` (used to pick the fanfare vs. plain game-over tone, and the game-over
screen text).

**`showGameOverScreen()`** — replaces the game view with "Game Over!" or
"New High Score!" plus `Sc:<score> Hi:<highScore>`.

**`setup()`** — initializes the LCD, builds the custom bird characters, sets pin
modes, seeds `random()` from the floating/unused `A5` pin (for less predictable pipe
placement), loads the high score from EEPROM, and resets game state.

**`loop()`** — the non-blocking main loop:
- Reads the joystick button with edge detection (`lastRestartButtonState`) to detect a
  single click rather than a held state.
- If `gameOver`, shows the game-over screen and restarts on a button click (with a
  short confirmation beep).
- Otherwise, advances the game only when `tickInterval` ms (260ms, i.e. difficulty)
  have elapsed since `lastTick`, using `millis()` rather than `delay()` so the button
  stays responsive between ticks. After `stepGame()`, plays the appropriate tone if the
  game just ended, then redraws.
