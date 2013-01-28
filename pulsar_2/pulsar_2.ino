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
//#define MODE_FADE               1
//#define MODE_ON                 2
#define MODE_LOW                1
#define MODE_MED                2
#define MODE_HIGH               3
#define MODE_PULSAR             4


// States
#define STATE_OFF               0
#define STATE_PULSE             1

// Timing
#define MAX_PULSE_MS         1000
#define MIN_PULSE_MS          100

// Button states
#define BUTTON_DOWN             0
#define BUTTON_UP               1


byte btnState;

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

int holdLength;

unsigned long stateBegan;
unsigned long time;
boolean newBtnDown;

int pulseLength;
int pulseChange = -50;

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
  
  pulseLength = MIN_PULSE_MS;
  
  btnState = BUTTON_UP;
  
  // Initialize serial busses
  // Serial.begin(9600);
  Wire.begin();
  // Serial.println("Ready.");
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
  state = STATE_PULSE;
  lightOn(255);
  stateBegan = millis();
}


void initWait() {
  lightOff();
  state = STATE_OFF;
  stateBegan = millis(); 
}

int getPulseLength() {
  return round(pulseLength / 2);
}

void pulse() {
  time = millis();
  int stateElap = time - stateBegan;
  float diff = startValue - endValue;
  float perComplete;
  
  if (time - lastTime > 100 && newBtnDown) {
    lastTime = time;
    if (pulseChange > 0 && pulseLength >= MAX_PULSE_MS) {
      pulseChange = -50;
    }
    if (pulseChange < 0 && pulseLength <= MIN_PULSE_MS) {
      pulseChange = 50;
    }
    
    pulseLength += pulseChange;
    // Serial.print("New Pulse Length: ");
    // Serial.println(pulseLength);
  }
  

  
  switch (state) {
    case STATE_PULSE:
      if (stateElap >= getPulseLength()) {
        initWait();
      }
      break;
    case STATE_OFF:
       if (stateElap >= getPulseLength()) {
        initPulse();
      }
      break;
      
  }
}

int lastHold() {
  time = millis();
  holdLength = time - btnTime;
  return holdLength;
}

boolean clickLength(int sm, int lg) {
  holdLength = lastHold();
  return (holdLength >= sm && holdLength <= lg);
} 

int downFor() {
  if (btnState == BUTTON_UP) {
    return lastHold();
  } else {
    return 0; 
  }
}

int upFor() {
  if (btnState == BUTTON_DOWN) {
    return lastHold();
  } else {
    return 0; 
  }
}


void loop() {
  time = millis();
  newBtnDown = digitalRead(DPIN_RLED_SW);
  

  //////////
  switch (mode) {
    case MODE_OFF:
      if (btnDown && !newBtnDown && clickLength(50, 250)) {
        mode = MODE_LOW;
        lightOn(85); 
        // Serial.println("Mode: LOW");
      }
      break;
    case MODE_LOW:
      lightOn(85);
      if (btnDown && !newBtnDown && clickLength(50, 250)) {
        mode = MODE_MED;
        lightOn(170);       
        // Serial.println("Mode: MEDIUM");
      }
      break;
    case MODE_MED:
      lightOn(170);
      if (btnDown && !newBtnDown && clickLength(50, 250)) {
        mode = MODE_HIGH;
        lightOn(255);
        // Serial.println("Mode: HIGH");
      }
      break;
    case MODE_HIGH:
      lightOn(255);
      if (btnDown && !newBtnDown && clickLength(50, 250)) {
        if (clickLength(50, 250)) {
          // Serial.println("Mode: PULSE");
          mode = MODE_PULSAR;
          initPulse();
        } else {
          // Serial.print("last: ");
          // Serial.print(btnDown ? 'DOWN' : 'UP');
          // Serial.print(", now: ");
          // Serial.print(newBtnDown ? 'DOWN' : 'UP');
          // Serial.print(", len: ");
          // Serial.println(lastHold());
        }
      }
      break;
    case MODE_PULSAR:
      pulse();
      if (btnDown && !newBtnDown && clickLength(50, 250)) {
        // Serial.println("Mode: OFF");
        mode = MODE_OFF;
        lightOff();
      }
      break;
  }
  
  if (newBtnDown != btnDown) {
    if (newBtnDown) {
      btnState = BUTTON_DOWN;
    } else {
      btnState = BUTTON_UP;
    }
    
    if (downFor() > 0) {
      // Serial.print("Click Length: ");
      // Serial.println(lastHold());
    }
    
    btnTime = time;
    btnDown = newBtnDown;
    delay(50);
  }
}

