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

extern "C" {
void TIMER1_IRQHandler(void) { IRQHandler(); }
}

void startTimer(void) {
  NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer; // Set the timer in Counter Mode
  NRF_TIMER1->TASKS_CLEAR = 1; // clear the task first to be usable for later
  NRF_TIMER1->PRESCALER = 4;
  NRF_TIMER1->BITMODE =
      TIMER_BITMODE_BITMODE_16Bit; // Set counter to 16 bit resolution
  NRF_TIMER1->CC[0] = 1000;        // Set value for TIMER2 compare register 0
  NRF_TIMER1->CC[1] = 0;           // Set value for TIMER2 compare register 1

  // Enable interrupt on Timer 2, both for CC[0] and CC[1] compare match events
  NRF_TIMER1->INTENSET =
      (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos);
  NVIC_EnableIRQ(TIMER1_IRQn);

  NRF_TIMER1->TASKS_START = 1; // Start TIMER2
}

void IRQHandler(void) {
  if ((NRF_TIMER1->EVENTS_COMPARE[0] != 0) &&
      ((NRF_TIMER1->INTENSET & TIMER_INTENSET_COMPARE0_Msk) != 0)) {
    NRF_TIMER1->EVENTS_COMPARE[0] = 0; // Clear compare register 0 event
    updateDisplay(framebuffer);//Only line changed from adafruit code.
    NRF_TIMER1->CC[0] += 1000;
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
  //set up outputs
  pinMode(mmCLK, OUTPUT);
  pinMode(mmDAT, OUTPUT);
  pinMode(srCLK, OUTPUT);
  pinMode(srLatch, OUTPUT);
  pinMode(srDAT, OUTPUT);
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
void printBuf(){
  for(int i=0;i<10;i++){
    Serial.print(framebuffer[i], BIN);
    Serial.print("\n");
  }
  Serial.print("\n");
}
//bmp[y] >> (width-1-x)) & 1
void drawChar(int c,int x, int y){
  if((x>9)||(x<-3)||(y>27)||(y<0)){
    return;
  }
  drawBitmap(x,y,tom_thumb_tall[c-32],8,4);
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

void scrollString(char text[], int y, int scrollSpeed){
  int textSize = strlen(text);
  for(int i=-10;i<(textSize*4);i++){
    for(int j=0;j<textSize;j++){
      if((-i+(4*j))>=0 && (-i+(4*j))<20){
        drawChar(text[j],-i+(4*j),y);
      }
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
