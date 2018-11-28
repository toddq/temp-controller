/*
 * Project TempController
 * Description:
 * Author:
 * Date:
 */

#include "LCD.h"
#include "TempSensor.h"
#include "RotaryEncoder.h"
#include "clickButton.h"

#define SENSOR_PIN D0
#define ROTARY_1 D2
#define ROTARY_2 D3
#define ROTARY_BUTTON D4
#define LED D7

#define MAGIC_NUMBER 2
#define SHORT_PRESS 1
#define LONG_PRESS -1

double setpoint = 60.0;
double currentTemp;
bool settingsMode = false;

// function declarations
int setSetpoint(String newSetpoint);
void saveValues();

struct SavedState {
  double setpoint;
  int magic_number;
};

// Initialize the LCD library - will write to pin TX
LCD lcd = LCD();
RotaryEncoder rotary(ROTARY_1, ROTARY_2);
ClickButton rotaryButton(ROTARY_BUTTON);
Timer saveTimer(5000, saveValues, true);

// setup() runs once, when the device is first turned on.
void setup() {
  // delay(2000);
  Time.zone(-8);
  Particle.syncTime();
  Serial.println("Starting up...");
  lcd.clear();
  lcd.line2(Time.timeStr());
  lcd.line1("Starting up...");

  initializeValues();
  // TODO: make sensor an object
  initSensor(SENSOR_PIN);
  initRotaryEncoder();
  pinMode(LED, OUTPUT);

  Particle.variable("temperature", currentTemp);
  Particle.variable("setpoint", setpoint);
  Particle.function("setpoint", setSetpoint);


  delay(3000);
  lcd.clear();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  updateTemp();
  // Serial.println(currentTemp);
  // probably change to %3f (drop decimal)
  if (!settingsMode) {
    lcd.line1(String::format("  Target: %4.1f F", setpoint));
    lcd.line2(String::format("  Actual: %4.1f F", currentTemp));
  }
  processRotaryButton();
  // delay(10);
}

void updateTemp() {
  static volatile unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000) {
    digitalWrite(LED, HIGH);
    Serial.println("updating temp...");
    currentTemp = getTemp();
    // TODO: publish
    lastUpdate = millis();
    digitalWrite(LED, LOW);
  }
}

int setSetpoint(String newSetpoint) {
  return _setSetpoint(newSetpoint.toFloat());
}

int _setSetpoint(double newSetpoint) {
  if (newSetpoint != setpoint) {
    setpoint = newSetpoint;
    saveTimer.startFromISR();
  }
  return 1;
}

void rotate() {
  int rotation = rotary.process();
  if (rotation == 0) {
    return;
  }

  if (settingsMode) {
    Serial.println("settings mode rotation");
  } else {
    // TODO: it's questionable if I should be setting this from an ISR
    double newSetpoint = setpoint + rotation;
    _setSetpoint(newSetpoint);
  }
}

void processRotaryButton() {
  rotaryButton.Update();
  if (rotaryButton.clicks == SHORT_PRESS) {
    if (settingsMode) {
      Serial.println("exit settings mode");
      settingsMode = false;
      lcd.clear();
    }
  } else if (rotaryButton.clicks == LONG_PRESS) {
    // setEditMode(true);
    if (!settingsMode) {
      Serial.println("enter settings mode");
      settingsMode = true;
      lcd.clear();
      lcd.line1("Settings:");
    }
  }
}

void initRotaryEncoder() {
    Serial.println("initializing rotary encoder");
    rotary.init(rotate);
}

void initializeValues() {
  long start = (long)millis();
  Serial.println("initializing values");
  SavedState value;
  EEPROM.get(0, value);
  Serial.println(value.magic_number);
  Serial.println(value.setpoint);
  if (value.magic_number == MAGIC_NUMBER) {
    setpoint = value.setpoint;
    Serial.printlnf("using stored setpoint %f", setpoint);
    long elapsed = (long)millis() - start;
    Serial.printlnf("read values in %d ms", elapsed);
  } else {
    Serial.println("using default value");
    saveValues();
  }
}

void saveValues() {
  long start = (long)millis();
  Serial.printlnf("Saving setpoint -  %f", setpoint);
  SavedState state = { setpoint,  MAGIC_NUMBER };
  Serial.printlnf("Size: %d", sizeof(state));
  EEPROM.put(0, state);
  long elapsed = (long)millis() - start;
  Serial.printlnf("wrote values in %d ms", elapsed);
}
