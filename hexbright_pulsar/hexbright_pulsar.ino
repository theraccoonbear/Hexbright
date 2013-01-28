/*
  Hexbright demo firmware: Fades
  Andrew Magill  9/2012
  
  1. Hold button down.  Light fades up and down.
  2. Let go of button.  Light holds current brightness.
  3. Press button.  Light turns off.
*/

#include <math.h>
#include <Wire.h>

// Pin assignments
#define DPIN_RLED_SW            2
#define DPIN_GLED               5
#define DPIN_PWR                8
#define DPIN_DRV_MODE           9
#define DPIN_DRV_EN             10
// Modes
#define MODE_OFF                0
#define MODE_FADE               1
#define MODE_ON                 2
#define MODE_PULSAR             3

// States
#define STATE_OFF               0
#define STATE_PULSE             1

// Timing
#define MAX_PULSE_MS          250;      
#define GAMMA_DELTA            20;

byte mode;
boolean btnDown;
unsigned long lastTime, btnTime;
int bright, fadeDir;

byte delay_time = 20;

int curChange;
int changeDir;

int startValue;
int curValue;
int endValue;
int change;
int waitTicks;
int ticks;
byte state;

unsigned long time;
boolean newBtnDown;



void setup()
{
  pinMode(DPIN_PWR,      INPUT);
  digitalWrite(DPIN_PWR, LOW);

  // Initialize GPIO
  pinMode(DPIN_RLED_SW,  INPUT);
  pinMode(DPIN_GLED,     OUTPUT);
  pinMode(DPIN_DRV_MODE, OUTPUT);
  pinMode(DPIN_DRV_EN,   OUTPUT);
  digitalWrite(DPIN_DRV_MODE, HIGH);
  digitalWrite(DPIN_DRV_EN,   LOW);
  digitalWrite(DPIN_GLED, HIGH);
  
  // Initialize serial busses
  Serial.begin(9600);
  Wire.begin();
  Serial.println("Ready.");
  curChange = -20;
  changeDir = 1;
  
  mode = MODE_OFF;
  btnDown = digitalRead(DPIN_RLED_SW);
  btnTime = millis();
}

void lightOff() {
  digitalWrite(DPIN_DRV_EN, LOW);
  pinMode(DPIN_PWR,      OUTPUT);
  digitalWrite(DPIN_PWR, LOW);
}

void lightOn(int val) {
  pinMode(DPIN_PWR,      OUTPUT);
  unsigned int gamma = (val*val)>>8;
  analogWrite(DPIN_DRV_EN, gamma); 
}


void initPulse() {
  startValue = 255;
  curValue = startValue;
  endValue = 0;
  int span = abs(startValue - endValue);
  int steps = span / GAMMA_DELTA;
  change = MAX_PULSE_MS / steps;
  change = 
  waitTicks = 40;
  ticks = 0;
  state = STATE_PULSE;
  Serial.println("Pulse started...");
}

void pulse() {
  unsigned long time = millis();
  ticks++;
  
  
  if (time-lastTime > 100 && newBtnDown) {
    lastTime = time;
    if (changeDir > 0 && curChange >= -5) {
      changeDir = -2;
    }
    if (changeDir < 0 && curChange <= -30) {
      changeDir = 2;
    }
    curChange += changeDir;
    Serial.print(curChange);
    Serial.print(" : ");
    Serial.println(changeDir);
  }
  
  switch (state) {
    case STATE_PULSE:
      lightOn(curValue);
      
      curValue += change;
      
      if ((change < 0 && curValue <= endValue) || (change > 0 && curValue >= endValue)) {
        Serial.println("Pulse complete. Waiting...");
        lightOff();
        ticks = 0;
        state = STATE_OFF;
      }
      break;
    case STATE_OFF:
      if (ticks >= waitTicks) {
        Serial.print("Waiting complete.  ");
        initPulse();
      }
      break;
      
  }
  //delay(5);
}

void loop()
{
  time = millis();
  newBtnDown = digitalRead(DPIN_RLED_SW);
  //////////
  switch (mode) {
    case MODE_OFF:
      //if (btnDown && !newBtnDown && (time-btnTime)>20) {
      if (newBtnDown) {
        mode = MODE_PULSAR;
        initPulse();
      }
      break;
    case MODE_PULSAR:
      pulse();
      if (btnDown && !newBtnDown && (time-btnTime)>20) {
        mode = MODE_OFF;
        lightOff();
      }
      break;
  }
  
  if (newBtnDown != btnDown)
  {
    btnTime = time;
    btnDown = newBtnDown;
    delay(50);
  }
}

