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
#define HEAT_OUTPUT A4
#define COOL_OUTPUT A5

#define MAGIC_NUMBER 2
#define SHORT_PRESS 1
#define LONG_PRESS -1

enum Mode {
  on_off, pid, pwm
};

double setpoint = 60.0;
double currentTemp;
Mode mode = on_off;
bool settingsMode = false;
bool isHeating = false;
bool isCooling = false;

// function declarations
int setSetpoint(String newSetpoint);
void saveValues();

struct SavedState {
  int magic_number;
  double setpoint;
  Mode mode;
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
  pinMode(HEAT_OUTPUT, OUTPUT);
  pinMode(COOL_OUTPUT, OUTPUT);

  Particle.variable("temperature", currentTemp);
  Particle.variable("setpoint", setpoint);
  // this doesn't work because it's an enum
  Particle.variable("mode", mode);
  Particle.function("setpoint", setSetpoint);
  Particle.function("mode", setMode);

  delay(3000);
  lcd.clear();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  processRotaryButton();
  updateTemp();
  processControlState();
  updateDisplay();
}

// TODO: better name?
void processControlState() {
  // TODO: good place for some polymorphism
  const double hysteresis = 1.0;
  switch(mode) {
    case on_off:
    default:
      if (isHeating && currentTemp >= setpoint) {
        // Serial.printlnf("%f is greater than %f", currentTemp, setpoint);
        toggleHeat(false);
      } 
      if (isCooling && currentTemp <= setpoint) {
        // Serial.printlnf("%f is less than %f", currentTemp, setpoint);
        toggleCooling(false);
      }
      if (currentTemp < (setpoint - hysteresis)) {
        // Serial.printlnf("%f is less than %f", currentTemp, (setpoint - hysteresis));
        toggleHeat(true);
      }
      if (currentTemp > (setpoint + hysteresis)) {
        // Serial.printlnf("%f is greater than %f", currentTemp, (setpoint + hysteresis));
        toggleCooling(true);
      }
  }
}

void updateDisplay() {
  // probably change to %3f (drop decimal)
  if (!settingsMode) {
    lcd.line1(String::format("  Target: %4.1f F", setpoint));
    lcd.line2(String::format("  Actual: %4.1f F", currentTemp));
  }
}

void toggleHeat(bool newState) {
  if (newState != isHeating) {
    Serial.printlnf("toggling heat %s", newState ? "on" : "off");
    isHeating = newState;
    digitalWrite(HEAT_OUTPUT, isHeating);
    // TODO: publish state
  }
}

void toggleCooling(bool newState) {
  if (newState != isCooling) {
    Serial.printlnf("toggling cooling %s", newState ? "on" : "off");
    isCooling = newState;
    digitalWrite(COOL_OUTPUT, isCooling);
    // TODO: publish state
  }
}

void updateTemp() {
  static volatile unsigned long lastUpdate = 0;
  if (!settingsMode && (millis() - lastUpdate > 5000)) {
    digitalWrite(LED, HIGH);
    // Serial.println("updating temp...");
    // TODO: ignore bad values
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

int setMode(String newMode) {
  return _setMode(newMode.toInt());
}

int _setMode(int newMode) {
  if (newMode != mode) {
    mode = static_cast<Mode>(newMode);
    saveTimer.startFromISR();
  }
  return 1;
}

String getModeString(Mode _mode) {
  return (const char *[]) {
    "On/Off",
    "PID",
    "PWM"
  }[_mode];
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
      lcd.line1("Mode:");
      lcd.line2(getModeString(mode));
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
    mode = value.mode;
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
  SavedState state = { MAGIC_NUMBER, setpoint, mode };
  Serial.printlnf("Size: %d", sizeof(state));
  EEPROM.put(0, state);
  long elapsed = (long)millis() - start;
  Serial.printlnf("wrote values in %d ms", elapsed);
}
