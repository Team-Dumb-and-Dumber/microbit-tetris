#ifndef MATRIX_DISPLAY_H
#define MATRIX_DISPLAY_H
#include <Arduino.h>

const int srLatch=20;//Default microBit pin numbers
const int mmDAT=19;  //Might end up changing these could interfere with matrix display
const int mmCLK=16;
const int srDAT=15;
const int srCLK=14;

const uint32_t srLatchmask = digitalPinToBitMask(srLatch);
const uint32_t mmDATmask = digitalPinToBitMask(mmDAT);
const uint32_t mmCLKmask = digitalPinToBitMask(mmCLK);
const uint32_t srDATmask = digitalPinToBitMask(srDAT);
const uint32_t srCLKmask = digitalPinToBitMask(srCLK);

//void TIMER2_IRQHandler(void);
void startTimer(void);
void IRQHandler(void);
void mmUpdate(uint32_t leds);
void srUpdate(uint32_t out);
void updateDisplay(uint32_t img[10]);
void setupDisplay();
void placeBlock(int blockType, int rotation, int xPos, int yPos);
void drawBitmap(int xPos, int yPos, byte bmp[], int width, int height);
void putPixel(int x, int y);
void clearDisplay();
void bodgeRefresh();
uint32_t * fbPointer();
void drawChar(int c,int x, int y);
void scrollString(char text[], int y, int scrollSpeed);
void printBuf();
void rotateBitmap(byte bmp[], byte* output, int height, int width);
#endif
