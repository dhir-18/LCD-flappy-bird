# LCD Flappy Bird

Flappy Bird on a 16x2 character LCD, built for the ELEGOO UNO starter kit
(Arduino Uno). Controlled with the kit's joystick module, with sound on an
active buzzer and a high score that persists across power cycles via EEPROM.

## Hardware

- Arduino Uno
- 16x2 character LCD (HD44780-compatible)
- Potentiometer (LCD contrast)
- Joystick module
- Active buzzer

See [WIRING.md](WIRING.md) for the full pinout.

## Controls

- Tilt the joystick up/down to move the bird. Release it to let the bird
  drift down under light gravity.
- Avoid the pipes scrolling in from the right.
- Click the joystick to restart after game over.
- Beating your high score plays a rising fanfare; the high score is saved to
  EEPROM and survives power-off.

## Setup

1. Wire the board as described in [WIRING.md](WIRING.md).
2. Open `flappy_bird_lcd.ino` in the Arduino IDE.
3. Select **Arduino Uno** as the board and the correct serial port.
4. Upload.

No external libraries are required — the sketch only uses `LiquidCrystal`
and `EEPROM`, both bundled with the Arduino IDE.
