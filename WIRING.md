# Wiring

Target board: Arduino Uno (ELEGOO UNO Starter Kit). All pin numbers below are
Arduino digital/analog pin numbers, taken from `flappy_bird_lcd.ino`.

## 16x2 character LCD (HD44780-compatible, 4-bit mode)

| LCD pin | Connects to          |
|---------|-----------------------|
| VSS     | GND                    |
| VDD     | 5V                     |
| V0      | Potentiometer wiper (contrast) |
| RS      | Arduino pin 12         |
| RW      | GND                     |
| E       | Arduino pin 11         |
| D4      | Arduino pin 5          |
| D5      | Arduino pin 4          |
| D6      | Arduino pin 3          |
| D7      | Arduino pin 2          |
| A (backlight +) | 5V (through the kit's backlight resistor, if present) |
| K (backlight -) | GND               |

D0-D3 are left unconnected — the sketch drives the LCD in 4-bit mode via
`LiquidCrystal lcd(12, 11, 5, 4, 3, 2);`.

The contrast potentiometer's two outer legs go to 5V and GND; the wiper
(middle leg) goes to LCD V0.

## Joystick module

| Joystick pin | Connects to    |
|--------------|-----------------|
| VCC          | 5V              |
| GND          | GND             |
| VRx          | Not connected (unused) |
| VRy          | Arduino pin A0  |
| SW           | Arduino pin 7   |

`SW` uses the Arduino's internal pull-up (`INPUT_PULLUP` in code), so it does
not need an external pull-up resistor. Pressing the joystick pulls the pin
LOW; it's used to restart the game after game over.

## Buzzer (active buzzer)

| Buzzer pin | Connects to   |
|------------|----------------|
| +          | Arduino pin 8  |
| -          | GND            |

## Notes

- Analog pin A5 is intentionally left floating and is read once at startup
  purely to seed the random number generator (`randomSeed(analogRead(A5))`).
  It is not part of the wiring and should not be connected to anything.
