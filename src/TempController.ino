/*
 * Project TempController
 * Description:
 * Author:
 * Date:
 */

#include "LCD.h"
#include "TempSensor.h"
#include "PWM.h"
#include "RotaryEncoder.h"
#include "clickButton.h"
#include "JsonParserGeneratorRK.h"

#define SENSOR_PIN D0
#define ROTARY_1 D2
#define ROTARY_2 D3
#define ROTARY_BUTTON D4
#define LED D7
#define HEAT_OUTPUT A4
#define COOL_OUTPUT A5

const int MAGIC_NUMBER = 2;
const int SHORT_PRESS = 1;
const int LONG_PRESS = -1;

const int SENSOR_READ_INTERVAL = 5000;
const int PUBLISH_INTERVAL = 5000;
const int COMPRESSOR_DELAY = 180000;
const int PWM_FREQUENCY = 2000;

// modes.  was an enum, but they don't work with Particle.variable()
const int on_off = 0;
const int pid = 1;
const int pwm = 2;

double setpoint = 60.0;
int percentPower = 100;
double currentTemp;
int mode = on_off;
bool settingsMode = false;
int isHeating = FALSE;
int isCooling = FALSE;
bool heatingEnabled = true;
bool coolingEnabled = true;

// function declarations
int setSetpoint(String newSetpoint);
void saveValues();
void leaveSettingsMode();

struct SavedState {
  int magic_number;
  double setpoint;
  int mode;
  bool heatingEnabled;
  bool coolingEnabled;
};

// Initialize the LCD library - will write to pin TX
LCD lcd = LCD();
Sensor sensor(SENSOR_PIN);
RotaryEncoder rotary(ROTARY_1, ROTARY_2);
ClickButton rotaryButton(ROTARY_BUTTON);
PWMControl pwmController = PWMControl(HEAT_OUTPUT, PWM_FREQUENCY);
Timer saveTimer(5000, saveValues, true);
Timer settingsModeTimer(30000, leaveSettingsMode, true);

// setup() runs once, when the device is first turned on.
void setup() {
  delay(2000);
  Time.zone(-8);
  Particle.syncTime();
  Serial.println("Starting up...");
  lcd.clear();
  lcd.line2(Time.timeStr());
  lcd.line1("Starting up...");

  initializeValues();
  sensor.init();
  initRotaryEncoder();

  pinMode(LED, OUTPUT);
  pinMode(HEAT_OUTPUT, OUTPUT);
  pinMode(COOL_OUTPUT, OUTPUT);

  Serial.println("Registring cloud variables");
  Particle.variable("mode", mode);
  Particle.variable("temperature", currentTemp);
  Particle.variable("setpoint", setpoint);
  Particle.variable("heatEnabled", heatingEnabled);
  Particle.variable("coolEnabled", coolingEnabled);
  
  Serial.println("Registring cloud functions");
  Particle.function("setpoint", setSetpoint);
  Particle.function("mode", setMode);
  Particle.function("heatEnabled", enableHeating);
  Particle.function("coolEnabled", enableCooling);

  delay(3000);
  Serial.println("Ready to go!");
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  processRotaryButton();
  updateTemp();
  processControlState();
  updateDisplay();
  publishState();
}

void publishState() {
  static volatile unsigned long lastUpdate = 0;
  unsigned long now = millis();
  if (now - lastUpdate > PUBLISH_INTERVAL) {
    JsonWriterStatic<256> json;
    {
      JsonWriterAutoObject obj(&json);
      json.setFloatPlaces(2);
      json.insertKeyValue("temperature", currentTemp);
      json.insertKeyValue("setpoint", setpoint);
      json.insertKeyValue("mode", mode);
      json.insertKeyValue("isHeating", isHeating);
      json.insertKeyValue("isCooling", isCooling);
      json.insertKeyValue("heatEnabled", heatingEnabled);
      json.insertKeyValue("coolEnabled", coolingEnabled);
    }
    String values = json.getBuffer();
    // workaround for weird generator behavior
    if (!values.endsWith("}")) {
      // Serial.print("Fixing json - ");
      // Serial.println(values);
      values.remove(values.lastIndexOf("}")+1);
    }

    // Serial.print("Publish: ");
    // Serial.println(values);
    Particle.publish("values", values, PRIVATE);
    lastUpdate = now;
  }
}

// TODO: better name?
void processControlState() {
  // TODO: good place for some polymorphism
  const double hysteresis = 1.0;
  switch(mode) {
    case pwm:
      pwmController.process();
      break;
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
    if (mode == pwm) {
      lcd.line1(String::format("Power:      %3d%%", (int)setpoint));
    } else {
      lcd.line1(String::format("  Target: %4.1f F", setpoint));
    }
    lcd.line2(String::format("  Actual: %4.1f F", currentTemp));
  }
}

void toggleHeat(bool newState) {
  if (heatingEnabled && newState != isHeating) {
    Serial.printlnf("toggling heat %s", newState ? "on" : "off");
    isHeating = newState;
    digitalWrite(HEAT_OUTPUT, isHeating);
  }
}

void toggleCooling(bool newState) {
  static volatile unsigned long lastCoolingCycle = 0;
  unsigned long now = millis();
  if (coolingEnabled && newState != isCooling) {
    // for compressor protection
    if (newState && lastCoolingCycle && (now - lastCoolingCycle < COMPRESSOR_DELAY)) {
      // Serial.println("compressor protection");
      // Serial.printlnf("Now: %d, lastCool: %d, delay: %d", now, lastCoolingCycle, COMPRESSOR_DELAY);
      return;
    }
    Serial.printlnf("toggling cooling %s", newState ? "on" : "off");
    isCooling = newState;
    digitalWrite(COOL_OUTPUT, isCooling);
    lastCoolingCycle = now;
  }
}

void updateTemp() {
  static volatile unsigned long lastUpdate = 0;
  static volatile unsigned long readStarted = 0;

  unsigned long now = millis();
  if (!readStarted && (now - lastUpdate) > SENSOR_READ_INTERVAL) {
    sensor.requestRead();
    readStarted = now;
  } else if (readStarted && (now - readStarted) > sensor.getReadTime()) {
    currentTemp = sensor.getTemp();
    lastUpdate = now;
    readStarted = 0;
  }
}

int setSetpoint(String newSetpoint) {
  return _setSetpoint(newSetpoint.toFloat());
}

int _setSetpoint(double newSetpoint) {
  if (mode == pwm) {
    newSetpoint = constrain(newSetpoint, 0, 100);
    pwmController.setDutyCycle(newSetpoint);
  }
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
    mode = newMode;
    saveTimer.startFromISR();
  }
  return 1;
}

String getModeString(int _mode) {
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
    settingsModeTimer.reset();
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
      leaveSettingsMode();
    }
  } else if (rotaryButton.clicks == LONG_PRESS) {
    // setEditMode(true);
    Serial.println("BUTTON LONG_PRESS");
    if (!settingsMode) {
      enterSettingsMode();
    }
  }
}

int enableHeating(String newState) {
  bool _newState = newState.toLowerCase() == "true";
  if (_newState != heatingEnabled) {
    if (_newState == false) {
      // make sure heat output is off if disabling
      toggleHeat(false);
    }
    // TODO: pass this into pwm controller
    // or... can I just change the pin mode?
    heatingEnabled = _newState;
    Serial.printlnf("Enable heating: %d", heatingEnabled);
    saveTimer.reset();
  }
  return 1;
}

int enableCooling(String newState) {
  bool _newState = newState.toLowerCase() == "true";
  if (_newState != coolingEnabled) {
    if (_newState == false) {
      // make sure cool output is off if disabling
      toggleCooling(false);
    }
    coolingEnabled = _newState;
    Serial.printlnf("Enable cooling: %d", coolingEnabled);
    saveTimer.reset();
  }
  return 1;
}

void enterSettingsMode() {
  Serial.println("enter settings mode");
  settingsMode = true;
  lcd.clear();
  lcd.line1("Mode:");
  lcd.line2(getModeString(mode));
  settingsModeTimer.reset();
}

void leaveSettingsMode() {
  Serial.println("exit settings mode");
  settingsMode = false;
  settingsModeTimer.stop();
  lcd.clear();
}

void initRotaryEncoder() {
    Serial.println("initializing rotary encoder");
    rotary.init(rotate);
}

void initializeValues() {
  unsigned long start = micros();
  Serial.println("initializing values");
  SavedState value;
  EEPROM.get(0, value);
  Serial.println(value.magic_number);
  Serial.println(value.setpoint);
  if (value.magic_number == MAGIC_NUMBER) {
    // TODO: just make value the global state holder
    setpoint = value.setpoint;
    mode = value.mode;
    heatingEnabled = value.heatingEnabled;
    coolingEnabled = value.coolingEnabled;
    Serial.printlnf("using stored setpoint %f", setpoint);
    unsigned long elapsed = micros() - start;
    Serial.printlnf("read values in %d microseconds", elapsed);
  } else {
    Serial.println("using default value");
    saveValues();
  }
}

void saveValues() {
  unsigned long start = micros();
  Serial.println("Saving values ");
  SavedState state = { MAGIC_NUMBER, setpoint, mode, heatingEnabled, coolingEnabled};
  Serial.printlnf("Size: %d", sizeof(state));
  EEPROM.put(0, state);
  unsigned long elapsed = micros() - start;
  Serial.printlnf("wrote values in %d microseconds", elapsed);
}
