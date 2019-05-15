#include "LCD.h"

#define BLANK_LINE "                "

// Constructor
LCD::LCD() {
  _line1 = "";
  _line2 = "";
}

void LCD::moveCursorLine(int line) {
    Serial1.write(254);
    Serial1.write(line == 1 ? 128 : 192);
}

void LCD::clearLine(int line) {
    moveCursorLine(line);
    if (line == 1) {
      _line1 = BLANK_LINE;
    } else if (line == 2) {
      _line2 = BLANK_LINE;
    }
    Serial1.write(BLANK_LINE);
    moveCursorLine(line);
}

void LCD::line1(String text) {
  if (_line1 != text) {
    writeLine(1, text);
    _line1 = text;
  }
}

void LCD::line2(String text) {
  if (_line2 != text) {
    writeLine(2, text);
    _line2 = text;
  }
}

void LCD::writeLine(int line, String text) {
  clearLine(line);
  Serial1.write(text);
}
