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

byte mode;
boolean btnDown;
unsigned long lastTime, btnTime;
int bright, fadeDir;

byte delay_time = 20;

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
  
  mode = MODE_OFF;
  btnDown = digitalRead(DPIN_RLED_SW);
  btnTime = millis();
}

void zoom(int first, int last, float delta) {
  float diff = first - last;
  float steps = round((first - last) / delta);
  steps = abs(steps);
  int bright = first;

  for (float i = 0; i < steps; i++) {
    unsigned int gamma = (bright*bright)>>8;
    if (gamma < 6) gamma = 6;
    Serial.print("gamma: ");
    Serial.println(gamma);
    
    analogWrite(DPIN_DRV_EN, gamma);
    bright += delta;
  }  
}

void pulse() {
  zoom(255, 0, -30);
  delay(50);
  zoom(255, 0, -30);
  delay(50);
  zoom(255, 0, -30);

  delay(250);
}

void loop()
{
  unsigned long time = millis();
  boolean newBtnDown = digitalRead(DPIN_RLED_SW);
  
  switch (mode)
  {
  case MODE_OFF:
    if (btnDown && newBtnDown && time-btnTime>50)
    {
      mode = MODE_FADE;
      bright = 0;
      fadeDir = 10;
      pinMode(DPIN_PWR,      OUTPUT);
      digitalWrite(DPIN_PWR, HIGH);
    }
    break;
  case MODE_FADE:
    //if (time-lastTime > 5)
//    if (1 == 1) {
//      lastTime = time;
//      if (fadeDir>0 && bright==255) fadeDir = -10;
//      if (fadeDir<0 && bright==0  ) fadeDir =  10;
//      bright += fadeDir;
//      unsigned int gamma = (bright*bright)>>8;
//      if (gamma < 6) gamma = 6;
//      Serial.print("gamma: ");
//      Serial.println(gamma);
//      analogWrite(DPIN_DRV_EN, gamma);
//    }
    pulse();
    if (!btnDown && !newBtnDown && time-btnTime>50)
      mode = MODE_ON;
    break;
  case MODE_ON:
    if (btnDown && newBtnDown && time-btnTime>200)
      mode = MODE_FADE;
    if (btnDown && !newBtnDown)
    {
      mode = MODE_OFF;
      digitalWrite(DPIN_DRV_EN, LOW);
      pinMode(DPIN_PWR,      OUTPUT);
      digitalWrite(DPIN_PWR, LOW);
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

