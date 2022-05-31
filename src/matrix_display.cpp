#include "matrix_display.h"
#include "matrix_font.h"

NRF_GPIO_Type* port;

uint32_t ledPos = 0b00000000000000000000;
uint32_t srPos = UINT32_MAX;
extern uint32_t framebuffer[10] ={
  0b00000000000000000000,
  0b00000000000000000000,
  0b00000000000000000000,
  0b00000000000000000000,
  0b00000000000000000000,
  0b00000000000000000000,
  0b00000000000000000000,
  0b00000000000000000000,
  0b00000000000000000000,
  0b00000000000000000000,
};

byte blocks[][4] = {//I,J,L,O,S,Z,T blocks
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
int blocksizes[][2] = {
/*
0 - I
1 - J
2 - L
3 - O
4 - S
5 - Z
6 - T
*/
  {1,4},
  {2,3},
  {2,3},
  {2,2},
  {2,3},
  {2,3},
  {2,3}
};

int gameOver;

byte newfont[95][8] = {0};
byte fullblocks[7][4][4] = {0};
int staticBlocks[100][4] = {0};//due to the nature of the game, there will only ever be one block moving at once
int activeBlock[4] = {0};//the one block that is moving
int blockCounter = 0;
long lastDrop;

extern "C" {
void TIMER2_IRQHandler(void) { IRQHandler(); }
}

void startTimer(void) {
  NRF_TIMER2->MODE = TIMER_MODE_MODE_Timer; // Set the timer in Counter Mode
  NRF_TIMER2->TASKS_CLEAR = 1; // clear the task first to be usable for later
  NRF_TIMER2->PRESCALER = 4;
  NRF_TIMER2->BITMODE =
      TIMER_BITMODE_BITMODE_16Bit; // Set counter to 16 bit resolution
  NRF_TIMER2->CC[0] = 1000;        // Set value for TIMER2 compare register 0
  NRF_TIMER2->CC[1] = 0;           // Set value for TIMER2 compare register 1

  // Enable interrupt on Timer 2, both for CC[0] and CC[1] compare match events
  NRF_TIMER2->INTENSET =
      (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos);
  NVIC_EnableIRQ(TIMER2_IRQn);

  NRF_TIMER2->TASKS_START = 1; // Start TIMER2
}

void IRQHandler(void) {
  if ((NRF_TIMER2->EVENTS_COMPARE[0] != 0) &&
      ((NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE0_Msk) != 0)) {
    NRF_TIMER2->EVENTS_COMPARE[0] = 0; // Clear compare register 0 event
    updateDisplay(framebuffer);//Only line changed from adafruit code.
    NRF_TIMER2->CC[0] += 1000;
  }
}

void mmUpdate(uint32_t leds){
  leds = leds << 12;//bit shift to left by 12, since we are driving 20 LEDs but use a 32 bit int
  port->OUTSET = mmDATmask; //set data pin to 1
  port->OUTSET = mmCLKmask; //set clock pin to 1
  port->OUTCLR = mmCLKmask; //turn the clock pin off
  for(int i=0;i<32;i++){
    if((leds >> (31-i)) & 1){//get bit i 
      port->OUTSET = mmDATmask;
      port->OUTSET = mmCLKmask;
      port->OUTCLR = mmCLKmask;
    }else{
      port->OUTCLR = mmDATmask;
      port->OUTSET = mmCLKmask;
      port->OUTCLR = mmCLKmask;
    }
  }
  /*
   * MM5450YN only latches after 36 bits have been shifted in, so we need to pad our 32 bit int with 4 zeros
   */
  port->OUTCLR = mmDATmask; //set mmDAT to 0
  for(int i=0;i<4;i++){//pulse clock 4 times
     port->OUTSET = mmCLKmask;
     port->OUTCLR = mmCLKmask;
  }
}

void srUpdate(uint32_t out){
  for(int i=0;i<16;i++){
    if((out >> (15-i)) & 1){//get value of bit i
      port->OUTSET = srDATmask;
      port->OUTSET = srCLKmask;
      port->OUTCLR = srCLKmask;
    }else{
      port->OUTCLR = srDATmask;
      port->OUTSET = srCLKmask;
      port->OUTCLR = srCLKmask;
    }
  }
  port->OUTSET = srLatchmask;
  port->OUTCLR = srLatchmask;
}

void updateDisplay(uint32_t img[10]){
  for(int i=0;i<10;i++){//i=column number
    srPos=UINT32_MAX; //
    srUpdate(srPos);
    if(i<5){ //first and last two pins of each shift registers are NC. This makes sure we shift the right thing to the registers.
      srPos &= ~(1 << (i+1)); //set bit of current column to 0. (the shift registers take inverted input)
    }else{
      srPos &= ~(1 << (i+4));
    }
    mmUpdate(img[i]);
    srUpdate(srPos);
  }
  srPos=UINT32_MAX;
  srUpdate(srPos);
}

void setupDisplay(){
  port = digitalPinToPort(mmDAT);
  gameOver=0;
  //set up outputs
  pinMode(mmCLK, OUTPUT);
  pinMode(mmDAT, OUTPUT);
  pinMode(srCLK, OUTPUT);
  pinMode(srLatch, OUTPUT);
  pinMode(srDAT, OUTPUT);
  pinMode(0,INPUT);
  generateBitmaps();
  port->OUTCLR = mmDATmask | mmCLKmask | srDATmask | srCLKmask | srLatchmask; //set all outputs to 0
  startTimer();
}

void drawBitmap(int xPos, int yPos, byte bmp[], int width, int height){
  int lim = height;
  if(height+xPos>10){
    lim = 10-xPos;
  }
  if((yPos)<20){
    for(int y=0;y<lim;y++){
      framebuffer[y+xPos] |= (uint32_t)bmp[y] << (19-yPos);
    }
  }else{
    for(int y=0;y<lim;y++){
      framebuffer[y+xPos] |= (uint32_t)bmp[y] >> (yPos-19);
    }
  }
}

void drawChar(int c,int x, int y){
  if((x>9)||(x<-3)||(y>27)||(y<0)){
    return;
  }
  drawBitmap(x,y,newfont[c-32],4,8);
}

void putPixel(int x, int y){
  if((x<10)&&(x>=0)&&(y>=0)&&(y<20)){
    framebuffer[x] |= 1 << (19-y);
  }
}

void clearDisplay(){
  for(int i=0;i<10;i++){
    framebuffer[i]=0;
  }
}

// {x,y,type,rotation}
void drawBlock(int blockdata[]){
  drawBitmap(blockdata[0], blockdata[1], fullblocks[blockdata[2]][blockdata[3]], 4, 4);
}

void testFunc(){
  Serial.print("Test\n");
}

void scrollString(char text[], int y, int scrollSpeed){
  int textSize = strlen(text);
  for(int i=-10;i<(textSize*8);i++){
    for(int j=0;j<textSize;j++){
      drawChar(text[j],y,(8*j)-i);
    }
  delay(scrollSpeed);
  clearDisplay();
  }
}

void rotateBitmap(byte bmp[], byte* output, int height, int width){
  for(int i=0;i<height;i++){
    for(int j=0;j<width;j++){
      if((bmp[i] >> j) & 1){
        output[width-j-1] |= 1 << i;
      }
    }
  }
}


void generateBitmaps(){
  
  int width, height, buf;
  for(int b=0;b<7;b++){
    for(int k=0;k<4;k++){
      fullblocks[b][0][k]=blocks[b][k];
    }
    width = blocksizes[b][0];
    height = blocksizes[b][1];
    buf = 0;
    for(int j=0;j<height;j++){
      byte tmpbmp[4] = {0,0,0,0};
      rotateBitmap(fullblocks[b][j], &tmpbmp[0],height,width);
      for(int i=0;i<4;i++){
        fullblocks[b][j+1][i]=tmpbmp[i];
      }
      buf = width;
      width = height;
      height = buf;
    }
  }
  for(int i=0;i<4;i++){
    fullblocks[0][2][i] = fullblocks[0][0][i];
    fullblocks[0][3][i] = fullblocks[0][1][i];
    fullblocks[3][3][i] = fullblocks[3][0][i];
  }
  //now we need to conform to standard tetris block rotations
  //described in https://strategywiki.org/wiki/File:Tetris_rotation_Nintendo.png
  //mostly just shifting around bitmaps. All blocks are square in size now, 4x4 for I, 3x3 for all others
}

void drawStaticBlocks(){
  for(int i=0;i<blockCounter;i++){
    drawBlock(staticBlocks[i]);
  }
}
//{x,y,blockType,Rotation}
void spawnBlock(){
  activeBlock[3]=random(4);
  activeBlock[2]=random(4);
  if(activeBlock[3]==0||activeBlock[3]==2){
    activeBlock[1]=19+blocksizes[activeBlock[2]][0];
    activeBlock[0]=random(10-blocksizes[activeBlock[2]][1]);
  }else{
    activeBlock[1]=19+blocksizes[activeBlock[2]][1];
    activeBlock[0]=random(10-blocksizes[activeBlock[2]][0]);
  }
}


void updatePhysics(){
  if((millis()-lastDrop)>60){//animate every 500ms 
    lastDrop=millis();
    clearDisplay();
    drawStaticBlocks();
    int bottomPix;
    if(activeBlock[3]==0|activeBlock[3]==2){
      bottomPix=activeBlock[1]-blocksizes[activeBlock[2]][0];
    }else{
      bottomPix=activeBlock[1]-blocksizes[activeBlock[2]][1];
    }
    if(bottomPix==-1){//if next block is the floor
      for(int i=0;i<4;i++){
        staticBlocks[blockCounter][i]=activeBlock[i];//make active block static
      }
      spawnBlock();
      blockCounter+=1;//increment block counter
      return;
    }
    if(checkCollisionsBitmap(activeBlock[0],activeBlock[1]-1,activeBlock[2],activeBlock[3])){//check collisions at next y position
      for(int i=0;i<4;i++){
        staticBlocks[blockCounter][i]=activeBlock[i];//make active block static
        activeBlock[i] = 0;//make active block 0
      }
      spawnBlock();
      blockCounter+=1;//increment block counter
      return;
    }
    activeBlock[1]--;//drop the block
    drawBlock(activeBlock);
  }
}

int gameState(){
  return gameOver;
}
int * returnActive(){
  return &activeBlock[0];
}

/*
Before I forget, the logic should work something like this 
main loop:
draw static blocks -> check collisions -> drop block by 1 (make static if collision)
if collision, generate new block
repeat
do some shit with user input too eventually
*/

//based on the logic for drawBitmap(), except it does not modify the framebuffer, only checks it against the active block bitmap
int checkCollisionsBitmap(int xPos, int yPos, int blockType, int rotation){
  int lim = 4;
  byte bmp[4];
  for(int i=0;i<4;i++){//This is dumb. Why cant c let me write an array to an array? dumb
    bmp[i] = fullblocks[blockType][rotation][i];
  }
  if(lim+xPos>10){
    lim = 10-xPos;
  }
  if((yPos)<20){
    for(int y=0;y<lim;y++){
      if((framebuffer[y+xPos]) & ((uint32_t)bmp[y] << (19-yPos))){//if the bitwise and of these two is non-zero, a collision must exist
        if(yPos>18){
          gameOver=1;
        }
        return(1);
      }
    }
  }else{
    for(int y=0;y<lim;y++){
      if((framebuffer[y+xPos]) & ((uint32_t)bmp[y] >> (yPos-19))){
        if(yPos>18){
          gameOver=1;
        }
        return(1);
      }
    }
  }
  return(0);
}

void mainGameLoop(){
  spawnBlock();
  while(!gameOver){
    updatePhysics();
  }
  clearDisplay();
  blockCounter=0;
  memset(staticBlocks, 0, sizeof(staticBlocks));
  gameOver=0;
}