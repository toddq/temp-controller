/*
 * Project TempController
 * Description:
 * Author:
 * Date:
 */

#include "LCD.h"
#include "TempSensor.h"

#define SENSOR_PIN D0
#define MAGIC_NUMBER 2

// Initialize the LCD library - will write to pin TX
LCD lcd = LCD();

double setpoint = 60.0;
double currentTemp;

struct SavedState {
  double setpoint;
  int magic_number;
};

int setSetpoint(String newSetpoint);

// setup() runs once, when the device is first turned on.
void setup() {
  // delay(2000);
  Serial.println("Starting up...");
  lcd.clear();
  lcd.line2(Time.timeStr());
  lcd.line1("Starting up...");

  initializeValues();

  // TODO: make sensor an object
  initSensor(SENSOR_PIN);

  Particle.variable("setpoint", setpoint);
  Particle.function("setpoint", setSetpoint);

  delay(3000);
  lcd.clear();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  currentTemp = getTemp();
  // Serial.println(currentTemp);
  // probably change to %3f (drop decimal)
  lcd.line1(String::format("  Target: %4.1f F", setpoint));
  lcd.line2(String::format("  Actual: %4.1f F", currentTemp));
  delay(1000);
  
}


int setSetpoint(String newSetpoint) {
  setpoint = newSetpoint.toFloat();
  saveValues();
  return 1;
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
