/*
  Flappy Bird on a 16x2 LCD - ELEGOO UNO Starter Kit version
  --------------------------------------------------------
  Wiring:
    LCD RS -> pin 12
    LCD E  -> pin 11
    LCD D4 -> pin 5
    LCD D5 -> pin 4
    LCD D6 -> pin 3
    LCD D7 -> pin 2
    LCD V0 -> potentiometer wiper (contrast)
    Joystick VRy -> A0
    Joystick SW  -> pin 7 (uses INPUT_PULLUP, restarts after game over)
    Joystick VCC -> 5V, GND -> GND (VRx not used)
    Buzzer -> pin 8 and GND (active buzzer)

  Controls:
    Tilt the joystick up/down to move the bird. Release to let it
    drift down under light gravity. Click the joystick to restart
    after a game over. Avoid the pipes scrolling in from the right.
    Your high score is saved to EEPROM and survives power-off, with
    a rising fanfare whenever you beat it.
*/

#include <LiquidCrystal.h>
#include <EEPROM.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int JOY_PIN = A0;
const int JOY_BUTTON_PIN = 7;
const int BUZZER_PIN = 8;
const int EEPROM_HIGH_SCORE_ADDR = 0;

const int SCREEN_COLS = 16;
const int BIRD_COL = 1;          // fixed horizontal column for the bird
const int MAX_POS = 15;          // 0..15 pixel-rows across both LCD rows
const int JOY_DEADZONE = 60;     // ignore small drift around center
const int JOY_MAX_SPEED = 2;     // max positions moved per tick from full tilt

int birdPos = 7;                 // current vertical position (0 top .. 15 bottom)
byte colState[SCREEN_COLS];      // 0 = empty, 1 = top blocked, 2 = bottom blocked

unsigned long lastTick = 0;
unsigned int tickInterval = 260; // ms between game steps (lower = harder)
int score = 0;
int highScore = 0;
bool isNewHighScore = false;
bool gameOver = false;

bool lastRestartButtonState = HIGH;

// 8 custom characters: each lights up one pixel-row (0-7) inside a cell
void createBirdChars() {
  for (int i = 0; i < 8; i++) {
    byte glyph[8] = {0,0,0,0,0,0,0,0};
    glyph[i] = 0b11111;
    lcd.createChar(i, glyph);
  }
}

void resetGame() {
  birdPos = 7;
  score = 0;
  isNewHighScore = false;
  gameOver = false;
  for (int i = 0; i < SCREEN_COLS; i++) colState[i] = 0;
  lastTick = millis();
}

void loadHighScore() {
  EEPROM.get(EEPROM_HIGH_SCORE_ADDR, highScore);
  // guard against garbage on a brand-new/unwritten EEPROM
  if (highScore < 0 || highScore > 999) highScore = 0;
}

void saveHighScore() {
  EEPROM.put(EEPROM_HIGH_SCORE_ADDR, highScore);
}

// Reads the joystick and returns how many positions to move the bird
// this tick: negative = up, positive = down. Returns 0 inside the deadzone.
int readJoystickVelocity() {
  int reading = analogRead(JOY_PIN);      // 0..1023, center ~512
  int deflection = reading - 512;
  if (abs(deflection) < JOY_DEADZONE) return 0;
  return map(deflection, -512, 512, -JOY_MAX_SPEED, JOY_MAX_SPEED);
}

void beep(int freq, int durationMs) {
  tone(BUZZER_PIN, freq, durationMs);
}

void gameOverTone() {
  tone(BUZZER_PIN, 200, 150);
  delay(150);
  tone(BUZZER_PIN, 120, 300);
}

void highScoreFanfare() {
  int notes[] = {523, 659, 784, 1047};   // C5, E5, G5, C6 - rising arpeggio
  int durations[] = {100, 100, 100, 250};
  for (int i = 0; i < 4; i++) {
    tone(BUZZER_PIN, notes[i], durations[i]);
    delay(durations[i] + 30);
  }
}

void drawGame() {
  lcd.clear();
  for (int c = 0; c < SCREEN_COLS; c++) {
    char topChar = ' ';
    char botChar = ' ';

    if (c == BIRD_COL) {
      // draw bird using the custom char matching its sub-position
      if (birdPos < 8) {
        topChar = (char)birdPos;
      } else {
        botChar = (char)(birdPos - 8);
      }
    } else {
      if (colState[c] == 1) topChar = (char)255;      // solid block
      else if (colState[c] == 2) botChar = (char)255; // solid block
    }

    lcd.setCursor(c, 0);
    lcd.print(topChar);
    lcd.setCursor(c, 1);
    lcd.print(botChar);
  }
}

void showGameOverScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (isNewHighScore) {
    lcd.print("New High Score!");
  } else {
    lcd.print("Game Over!");
  }
  lcd.setCursor(0, 1);
  lcd.print("Sc:");
  lcd.print(score);
  lcd.print(" Hi:");
  lcd.print(highScore);
}

void stepGame() {
  // score a point when a pipe fully exits on the left
  if (colState[0] != 0) score++;

  // scroll everything left
  for (int i = 0; i < SCREEN_COLS - 1; i++) {
    colState[i] = colState[i + 1];
  }

  // generate a new column at the right edge
  int roll = random(0, 100);
  if (roll < 35) {
    colState[SCREEN_COLS - 1] = (random(0, 2) == 0) ? 1 : 2;
  } else {
    colState[SCREEN_COLS - 1] = 0;
  }

  // joystick control, with a light gravity drift when centered
  int velocity = readJoystickVelocity();
  if (velocity == 0) velocity = 1; // gentle fall if not actively steering
  birdPos += velocity;

  if (birdPos > MAX_POS) {
    birdPos = MAX_POS;
    gameOver = true;
    checkHighScore();
    return;
  }
  if (birdPos < 0) birdPos = 0;

  // collision check at the bird's fixed column
  byte here = colState[BIRD_COL];
  if (here == 1 && birdPos < 8) gameOver = true;        // top half blocked
  if (here == 2 && birdPos >= 8) gameOver = true;        // bottom half blocked
  if (gameOver) checkHighScore();
}

void checkHighScore() {
  if (score > highScore) {
    highScore = score;
    isNewHighScore = true;
    saveHighScore();
  }
}

void setup() {
  lcd.begin(16, 2);
  createBirdChars();
  pinMode(JOY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  randomSeed(analogRead(A5)); // unused pin, for a less predictable seed
  loadHighScore();
  resetGame();
}

void loop() {
  bool buttonState = digitalRead(JOY_BUTTON_PIN);
  bool clicked = (lastRestartButtonState == HIGH && buttonState == LOW);
  lastRestartButtonState = buttonState;

  if (gameOver) {
    showGameOverScreen();
    if (clicked) {
      beep(600, 100);
      delay(150);
      resetGame();
    }
    return;
  }

  if (millis() - lastTick >= tickInterval) {
    lastTick = millis();
    stepGame();
    if (gameOver) {
      if (isNewHighScore) {
        highScoreFanfare();
      } else {
        gameOverTone();
      }
    }
    drawGame();
  }
}