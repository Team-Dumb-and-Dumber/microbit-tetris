#include "matrix_display.h"
#include "beemovie.h"
//#include <Adafruit_Microbit.h>
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

//Adafruit_Microbit_Matrix microbit;

byte gattybadatmaths[] = {
                          0b00000001,
                          0b00000011,
                          0b00000001,
                          0b00000000,
                          0b00000000,
                         };

int blocksdata[][4] ={
  {2,10,1,0},
  {5,10,3,0},
  {4,2,5,0}
};

byte testbmps[][4] = {//I,J,L,O,S,Z,T blocks
/*
0 - I
1 - J
2 - L
3 - O
4 - S
5 - Z
6 - T
*/
  {0b000000001,
   0b000000001,
   0b000000001,
   0b000000001},
  {0b000000011,
   0b000000001,
   0b000000001,
   0b000000000},
  {0b000000011,
   0b000000010,
   0b000000010,
   0b000000000},
  {0b000000011,
   0b000000011,
   0b000000000,
   0b000000000},
  {0b000000010,
   0b000000011,
   0b000000001,
   0b000000000},
  {0b000000001,
   0b000000011,
   0b000000010,
   0b000000000},
  {0b000000001,
   0b000000011,
   0b000000001,
   0b000000000}
};

byte rotationout[4] = {0};
byte rotationouttwo[4] = {0};
byte bmpstates[4][4] = {0};
byte bigbmparr[4][4] = {
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
  };
int currentBlock[4];
void setup() {
  Serial.begin(250000);
  setupDisplay();
  clearDisplay();
  for(int i=0;i<4;i++){
    bigbmparr[0][i]=testbmps[2][i];
  }
  int width = 2;
  int height = 3;
  int buf = 0;
  for(int j=0;j<3;j++){
    byte tmpbmp[4] = {0,0,0,0};
    rotateBitmap(bigbmparr[j], &tmpbmp[0],height,width);
    for(int i=0;i<4;i++){
      bigbmparr[j+1][i]=tmpbmp[i];
    }
    buf = width;
    width = height;
    height = buf;
  }
  randomSeed(analogRead(0));
}



void loop() {
//  mainGameLoop();

}
