/*
 * Project TempController
 * Description:
 * Author:
 * Date:
 */

#include "LCD.h"

// Initialize the LCD library - will write to pin TX
LCD lcd = LCD();

// setup() runs once, when the device is first turned on.
void setup() {
  delay(2000);
  lcd.clear();
  lcd.line2(Time.timeStr());
  lcd.line1("Starting up...");

  delay(3000);
  lcd.clear();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  lcd.line1("Hello World");
  delay(1000);
  lcd.clear();
  delay(1000);
}
