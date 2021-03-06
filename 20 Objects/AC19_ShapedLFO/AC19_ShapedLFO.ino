//  ============================================================
//
//  Program: ArdCore Waveshaped LFO
//
//  Description: Based on the speed of the LFO and a "warp"
//               setting, create a triangle-ish waveform out
//               the 8-bit output
//
//  I/O Usage:
//    Knob 1: LFO speed
//    Knob 2: LFO warp (0=normal triangle)
//    Analog In 1: unused
//    Analog In 2: unused
//    Digital Out 1: trigger at negative transition
//    Digital Out 2: trigger at positive transition
//    Clock In: unused
//    Analog Out: 8-bit output
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  12 Feb 2011
//  Modified: 18 Mar 2011 - ddg Complete rewrite of output
//                              Added trigger outputs
//            17 Apr 2012  ddg Updated for Arduino 1.0
//						18 Apr 2012	 ddg Changed dacOutput routine to Alba version
//
//  ============================================================
//
//  License:
//
//  This software is licensed under the Creative Commons
//  "Attribution-NonCommercial license. This license allows you
//  to tweak and build upon the code for non-commercial purposes,
//  without the requirement to license derivative works on the
//  same terms. If you wish to use this (or derived) work for
//  commercial work, please contact 20 Objects LLC at our website
//  (www.20objects.com).
//
//  For more information on the Creative Commons CC BY-NC license,
//  visit http://creativecommons.org/licenses/
//
//  ================= start of global section ==================

//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 25;       // ms for trigger time

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
unsigned long digMilli[2] = {0,0};       // last trigger time

//  variables used for clocking and stepwise movement
unsigned long lastMillis = 0;  // stored last millis test
float upStep = 1.0;            // value update per millisecond (up)
float downStep = 1.0;          // value update per millisecond (down)
float currValue = 0.0;         // the current LFO value
int currDir = 1;               // 1 for up, 0 for down

//  ==================== start of setup() ======================

void setup() {
  Serial.begin(9600);
  
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  
  // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], LOW);
  }
  
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }  
  
  getUpdateValue();
}

//  ==================== start of loop() =======================

void loop()
{
  // do an output
  float lastValue = currValue;
  unsigned long currMillis = millis();
  int timediff = currMillis - lastMillis;
  
  if (currValue > 255.0) {
    currDir = 0;
  } else {
    currDir = 1;
  }
  
  if (currDir) {
    currValue = currValue + (timediff * upStep);
  } else {
    currValue = currValue + (timediff * downStep);
  }
  
  while (currValue > 511.0) {
    currValue -= 511.0;
  }
  
  if (currValue <= 255.0) {
    dacOutput(int(currValue));
  } else {
    dacOutput(int(511.0 - currValue));
  }
  
  lastMillis = currMillis;
  
  // check for digital pin firing
  if ((lastValue <= 255.) && (currValue > 255.)) {
    digState[0] = HIGH;
    digitalWrite(digPin[0], HIGH);
    digMilli[0] = currMillis;
  }
  
  if (lastValue > currValue) {
    digState[1] = HIGH;
    digitalWrite(digPin[1], HIGH);
    digMilli[1] = currMillis;
  }
  
  // check for pin turn-off
  for (int i=0; i<2; i++) {
    if ((digState[i] == HIGH) && (currMillis - digMilli[i] > trigTime)) {
      digState[i] = LOW;
      digitalWrite(digPin[i], LOW);
    }
  }
  
  // check for new values
  getUpdateValue();
}

void getUpdateValue()
{
  float msPerCycle = ((1023 - analogRead(0)) + 20) * 3.0;
  float warpFactor = ((analogRead(1) >> 4) + 1) / 65.0;
  float oneOver = 1.0 - warpFactor;
  
  upStep = 255.0 / (msPerCycle * warpFactor);
  downStep = 255.0 / (msPerCycle * oneOver);
}

//  =================== convenience routines ===================

//  dacOutput(byte) - deal with the DAC output
//  -----------------------------------------
void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
	PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}

//  ===================== end of program =======================
