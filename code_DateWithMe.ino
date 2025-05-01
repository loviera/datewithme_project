#include <Arduino.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES   4
#define CS_PIN        10

MD_Parola P(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
MD_MAX72XX mx(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Buttons on D2..D5
const uint8_t btnUp=2, btnDown=3, btnLeft=4, btnRight=5;
// Dot position
int dotX=15, dotY=3;
// Box and X positions
const int yesBoxX=8, boxY=4;
int noX=20, noY=4;  // X-shaped choice uses same top-left coordinate
// Debounce
unsigned long lastDebounce=0;
const unsigned long DEBOUNCE_MS=50;
bool lastUp, lastDown, lastLeft, lastRight;
// State machine
enum State{SCROLL, MENU, SHOW} state=SCROLL;

void setup(){
  Serial.begin(115200);
  pinMode(btnUp, INPUT_PULLUP);
  pinMode(btnDown, INPUT_PULLUP);
  pinMode(btnLeft, INPUT_PULLUP);
  pinMode(btnRight, INPUT_PULLUP);
  lastUp=digitalRead(btnUp);
  lastDown=digitalRead(btnDown);
  lastLeft=digitalRead(btnLeft);
  lastRight=digitalRead(btnRight);

  // Initialize Parola for scrolling
  P.begin(); P.setIntensity(5);
  P.displayText("DATE WITH ME?", PA_LEFT, 75, 2000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  // Initialize raw matrix
  mx.begin(); mx.control(MD_MAX72XX::INTENSITY,5); mx.clear();
}

void loop(){
  unsigned long now=millis();
  switch(state){
    case SCROLL:
      if(P.displayAnimate()){
        Serial.println(">> SCROLL DONE");
        P.displayReset(); mx.clear();
        state=MENU;
      }
      break;
    case MENU:
      handleInput(now);
      drawMenu();
      break;
    case SHOW:
      // Scroll "PLEASE TEXT ME :D"
      P.displayText("TEXT ME :D", PA_LEFT, 75, 2000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      state=SCROLL;
      break;
  }
}

void handleInput(unsigned long now){
  #define CHK(btn,last,coord,d,minv,maxv) { bool cur=digitalRead(btn); if(cur!=last && now-lastDebounce>DEBOUNCE_MS){ lastDebounce=now; if(cur==LOW) coord=constrain(coord+d,minv,maxv);} last=cur; }
  CHK(btnUp,    lastUp,    dotY, -1, 0,7);
  CHK(btnDown,  lastDown,  dotY, +1, 0,7);
  // swap left/right
  CHK(btnLeft,  lastLeft,  dotX, +1, 0,31);
  CHK(btnRight, lastRight, dotX, -1, 0,31);
  // YES collision
  if(dotX>=yesBoxX && dotX<=yesBoxX+2 && dotY>=boxY && dotY<=boxY+2){
    Serial.println(">> HIT YES"); state=SHOW;
  }
  // NO collision
  if(dotX>=noX && dotX<=noX+2 && dotY>=noY && dotY<=noY+2){
    noX=random(0,29);
    Serial.println(">> EVADE NO");
  }
}

void drawMenu(){
  mx.clear();
  // Draw YES box (3x3 hollow)
  drawBox(yesBoxX, boxY);
  // Draw NO as 3x3 X shape
  drawX(noX, noY);
  // Draw dot
  mx.setPoint(dotY, dotX, true);
  mx.update();
}

void drawBox(int x0,int y0){
  for(int i=0;i<3;i++){
    mx.setPoint(y0,   x0+i, true);
    mx.setPoint(y0+2, x0+i, true);
  }
  mx.setPoint(y0+1, x0,     true);
  mx.setPoint(y0+1, x0+2,   true);
}

void drawX(int x0,int y0){
  // X shape 3x3
  mx.setPoint(y0,   x0,   true);
  mx.setPoint(y0+1, x0+1, true);
  mx.setPoint(y0+2, x0+2, true);
  mx.setPoint(y0,   x0+2, true);
  mx.setPoint(y0+1, x0+1, true);
  mx.setPoint(y0+2, x0,   true);
}
