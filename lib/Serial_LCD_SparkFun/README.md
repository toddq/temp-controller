_This is a port of the wonderful library found here: http://playground.arduino.cc/Code/SerLCD
The documentation there will explain all the functions available, and I expect them all to work
EXCEPT when it says `lcd.print()` If you're copying example code, you'll need to change those to
`Serial1.print()`... and that's it!_

The wiring is simple- Provide appropriate power and ground (there are 5v and 3.3v varieties
of the LCD) and connect the LCD's `RX` pin to the Core or Photon's `TX` pin!_

## Examples

1. __Basic use__ - A very short example showing LCD clearing and printing
2. __Internet display__ - Using Spark.function() to send messages to the LCD