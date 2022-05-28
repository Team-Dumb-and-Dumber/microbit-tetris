#include "matrix_display.h"
#include "beemovie.h"
#include <Adafruit_Microbit.h>
#include <Arduino.h>
/*ON PCB - ON MICROBIT
1 - LATCH - 20
2 - LED DRIVER DATA - 19
3 - LED DRIVER CLOCK - 16
4 - +5V
5 - GND
6 - SHIFT REG DATA - 15
7 - SHIFT REG CLOCK - 14
*/

Adafruit_Microbit_Matrix microbit;

byte gattybadatmaths[] = {
                          0b00000001,
                          0b00000011,
                          0b00000001,
                          0b00000000,
                          0b00000000,
                         };

void setup() {
  setupDisplay();
}

void loop() {
  drawBitmap(5,10,gattybadatmaths, 2, 3);
  clearDisplay();
}
