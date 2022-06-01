#ifndef MATRIX_DISPLAY_H
#define MATRIX_DISPLAY_H
#include <Arduino.h>

const int srLatch=20;//Default microBit pin numbers
const int mmDAT=19;  //Might end up changing these could interfere with matrix display
const int mmCLK=16;
const int srDAT=15;
const int srCLK=14;
const int buttons[6] = {13,2,11,8,1,5};

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
void drawBitmap(int xPos, int yPos, byte bmp[], int width, int height, uint32_t * outputframebuffer);
void putPixel(int x, int y);
void clearDisplay();
void drawChar(int c,int x, int y);
void scrollString(char text[], int y, int scrollSpeed);
void rotateBitmap(byte bmp[], byte* output, int height, int width);
void drawBlock(int blockdata[], uint32_t * outputbuffer);
void generateBitmaps();
void updatePhysics();
void spawnBlock();
void drawStaticBlocks();
void mainGameLoop();
void debounceButtons();
void addToStatic(int block[]);
void moveLeft();
void moveRight();
void handleButtons();
void rotateClk();
void rotateAClk();
int gameState();
int * returnActive();
int checkCollisionsBitmap(int xPos, int yPos, int blockType, int rotation);
#endif
