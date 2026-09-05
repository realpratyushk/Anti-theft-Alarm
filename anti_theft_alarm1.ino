#include <LiquidCrystal.h>
#include <Keypad.h>

// =====================================================
// LCD - NORMAL 16x2
// RS, E, D4, D5, D6, D7
// =====================================================

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);


// =====================================================
// SECURITY SETTINGS
// =====================================================

const String CORRECT_PIN = "2580";

const unsigned long WARNING_TIME = 3000UL;   // 5 sec
const unsigned long SILENT_TIME  = 10000UL;  // 10 sec
const unsigned long LOCK_TIME    = 30000UL;  // 30 sec


// =====================================================
// COMPONENT PINS
// =====================================================

const byte PIR_PIN    = 2;
const byte BUZZER_PIN = 10;
const byte RED_LED    = 11;
const byte GREEN_LED  = 13;


// =====================================================
// 4x4 KEYPAD
// =====================================================

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {3, 4, 5, 6};
byte colPins[COLS] = {7, 8, 9, 12};

Keypad keypad = Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  ROWS,
  COLS
);


// =====================================================
// SYSTEM STATES
// =====================================================

enum SystemState {
  ARMED,
  WARNING,
  SILENT,
  ALARM,
  LOCKED
};

SystemState state = ARMED;


// =====================================================
// VARIABLES
// =====================================================

String enteredPIN = "";

byte wrongAttempts = 0;

unsigned long stateStartTime = 0;

int lastSecond = -1;

unsigned long lastBlinkTime = 0;

bool ledState = false;


// =====================================================
// SETUP
// =====================================================

void setup() {

  pinMode(PIR_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  // Start LCD
  lcd.begin(16, 2);

  // Startup screen
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("ANTI THEFT");

  lcd.setCursor(0, 1);
  lcd.print("SYSTEM STARTING");

  delay(2000);

  setArmed();
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  checkKeypad();

  checkSystem();

}


// =====================================================
// KEYPAD
// =====================================================

void checkKeypad() {

  char key = keypad.getKey();

  if (!key) {
    return;
  }

  // Keypad disabled while locked
  if (state == LOCKED) {
    return;
  }


  // =================================================
  // CLEAR PIN
  // =================================================

  if (key == '*') {

    enteredPIN = "";

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("PIN CLEARED");

    delay(700);

    showPinScreen();

    return;
  }


  // =================================================
  // SUBMIT PIN
  // =================================================

  if (key == '#') {

    checkPIN();

    return;
  }


  // =================================================
  // NUMBER ENTRY
  // =================================================

  if (key >= '0' && key <= '9') {

    if (enteredPIN.length() < 8) {

      enteredPIN += key;

      showEnteredPIN();
    }
  }
}


// =====================================================
// SHOW ENTERED PIN
// =====================================================

void showEnteredPIN() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("ENTER PIN:");

  lcd.setCursor(0, 1);

  for (byte i = 0; i < enteredPIN.length(); i++) {

    lcd.print("*");

  }
}


// =====================================================
// CHECK PIN
// =====================================================

void checkPIN() {

  // =================================================
  // CORRECT PIN
  // =================================================

  if (enteredPIN == CORRECT_PIN) {

    enteredPIN = "";

    wrongAttempts = 0;

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("PIN CORRECT");

    lcd.setCursor(0, 1);
    lcd.print("ACCESS GRANTED");

    delay(1000);


    // Correct PIN during warning/alarm
    // activates silent mode

    if (state == WARNING || state == ALARM) {

      startSilent();

    }

    else {

      setArmed();

    }

    return;
  }


  // =================================================
  // WRONG PIN
  // =================================================

  wrongAttempts++;

  enteredPIN = "";

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("PIN INCORRECT");

  lcd.setCursor(0, 1);
  lcd.print("TRY: ");
  lcd.print(wrongAttempts);
  lcd.print("/3");


  // =================================================
  // 3 WRONG ATTEMPTS
  // =================================================

  if (wrongAttempts >= 3) {

    delay(1000);

    startLocked();

  }

  else {

    delay(1000);

    if (state == WARNING) {

      showWarningScreen();

    }

    else {

      showPinScreen();

    }
  }
}


// =====================================================
// CHECK SYSTEM STATE
// =====================================================

void checkSystem() {

  bool motion = digitalRead(PIR_PIN) == HIGH;


  // =================================================
  // ARMED
  // =================================================

  if (state == ARMED) {

    digitalWrite(BUZZER_PIN, LOW);

    digitalWrite(RED_LED, LOW);

    digitalWrite(GREEN_LED, HIGH);


    if (motion) {

      startWarning();

    }
  }


  // =================================================
  // WARNING
  // =================================================

  else if (state == WARNING) {

    digitalWrite(BUZZER_PIN, LOW);

    digitalWrite(GREEN_LED, LOW);

    blinkRed();


    unsigned long elapsed =
      millis() - stateStartTime;


    // Motion stopped before alarm
    if (!motion) {

      setArmed();

      return;
    }


    // 5 seconds completed
    if (elapsed >= WARNING_TIME) {

      startAlarm();

      return;
    }


    updateWarningCountdown();
  }


  // =================================================
  // SILENT MODE
  // =================================================

  else if (state == SILENT) {

    digitalWrite(BUZZER_PIN, LOW);

    digitalWrite(RED_LED, LOW);

    blinkGreen();


    unsigned long elapsed =
      millis() - stateStartTime;


    // 10 seconds completed
    if (elapsed >= SILENT_TIME) {

      setArmed();

      return;
    }


    unsigned long remaining =
      SILENT_TIME - elapsed;


    int secondsLeft =
      (remaining + 999) / 1000;


    if (secondsLeft != lastSecond) {

      lastSecond = secondsLeft;

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("SILENT MODE");

      lcd.setCursor(0, 1);
      lcd.print("TIME LEFT:");

      lcd.print(secondsLeft);

      lcd.print(" SEC");
    }
  }


  // =================================================
  // ALARM
  // =================================================

  else if (state == ALARM) {

    digitalWrite(GREEN_LED, LOW);

    digitalWrite(RED_LED, HIGH);

    digitalWrite(BUZZER_PIN, HIGH);


    // Stop alarm when motion stops
    if (!motion) {

      setArmed();

    }
  }


  // =================================================
  // LOCKED
  // =================================================

  else if (state == LOCKED) {

    // IMPORTANT:
    // Buzzer stays ON even without motion
    digitalWrite(BUZZER_PIN, HIGH);

    // Green LED OFF
    digitalWrite(GREEN_LED, LOW);

    // Red LED blinking
    blinkRed();


    unsigned long elapsed =
      millis() - stateStartTime;


    // 30 seconds completed
    if (elapsed >= LOCK_TIME) {

      wrongAttempts = 0;

      digitalWrite(BUZZER_PIN, LOW);

      setArmed();

      return;
    }


    unsigned long remaining =
      LOCK_TIME - elapsed;


    int secondsLeft =
      (remaining + 999) / 1000;


    if (secondsLeft != lastSecond) {

      lastSecond = secondsLeft;

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("SYSTEM LOCKED");

      lcd.setCursor(0, 1);
      lcd.print("WAIT:");

      lcd.print(secondsLeft);

      lcd.print(" SEC");
    }
  }
}


// =====================================================
// START WARNING
// =====================================================

void startWarning() {

  state = WARNING;

  stateStartTime = millis();

  lastSecond = -1;

  enteredPIN = "";

  digitalWrite(BUZZER_PIN, LOW);

  digitalWrite(GREEN_LED, LOW);

  showWarningScreen();
}


// =====================================================
// WARNING SCREEN
// =====================================================

void showWarningScreen() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("MOTION DETECTED");

  lcd.setCursor(0, 1);
  lcd.print("ENTER PIN!");
}


// =====================================================
// WARNING COUNTDOWN
// =====================================================

void updateWarningCountdown() {

  unsigned long elapsed =
    millis() - stateStartTime;


  int secondsLeft =
    (WARNING_TIME - elapsed + 999) / 1000;


  if (secondsLeft != lastSecond) {

    lastSecond = secondsLeft;

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("MOTION DETECTED");

    lcd.setCursor(0, 1);
    lcd.print("WARNING:");

    lcd.print(secondsLeft);

    lcd.print(" SEC");
  }
}


// =====================================================
// START SILENT MODE
// =====================================================

void startSilent() {

  state = SILENT;

  stateStartTime = millis();

  lastSecond = -1;

  enteredPIN = "";

  digitalWrite(BUZZER_PIN, LOW);

  digitalWrite(RED_LED, LOW);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("PIN CORRECT");

  lcd.setCursor(0, 1);
  lcd.print("SILENT MODE");

  delay(1000);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SILENT MODE");

  lcd.setCursor(0, 1);
  lcd.print("TIME LEFT: 10");
}


// =====================================================
// START ALARM
// =====================================================

void startAlarm() {

  state = ALARM;

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("!!! ALARM !!!");

  lcd.setCursor(0, 1);
  lcd.print("MOTION DETECTED");
}


// =====================================================
// START LOCK
// =====================================================

void startLocked() {

  state = LOCKED;

  stateStartTime = millis();

  lastSecond = -1;

  enteredPIN = "";

  // Buzzer ON immediately
  digitalWrite(BUZZER_PIN, HIGH);

  digitalWrite(GREEN_LED, LOW);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SYSTEM LOCKED");

  lcd.setCursor(0, 1);
  lcd.print("WAIT: 30 SEC");
}


// =====================================================
// ARMED MODE
// =====================================================

void setArmed() {

  state = ARMED;

  stateStartTime = millis();

  lastSecond = -1;

  enteredPIN = "";

  digitalWrite(BUZZER_PIN, LOW);

  digitalWrite(RED_LED, LOW);

  digitalWrite(GREEN_LED, HIGH);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("ANTI THEFT");

  lcd.setCursor(0, 1);
  lcd.print("SYSTEM ARMED");
}


// =====================================================
// PIN SCREEN
// =====================================================

void showPinScreen() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SYSTEM ARMED");

  lcd.setCursor(0, 1);
  lcd.print("ENTER PIN:");
}


// =====================================================
// RED LED BLINK
// =====================================================

void blinkRed() {

  if (millis() - lastBlinkTime >= 200) {

    lastBlinkTime = millis();

    ledState = !ledState;

    digitalWrite(RED_LED, ledState);
  }
}


// =====================================================
// GREEN LED BLINK
// =====================================================

void blinkGreen() {

  if (millis() - lastBlinkTime >= 300) {

    lastBlinkTime = millis();

    ledState = !ledState;

    digitalWrite(GREEN_LED, ledState);
  }
}