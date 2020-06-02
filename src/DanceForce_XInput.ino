
/*  
 *  Charles Scheidecker for Sparkfun Pro Micro board with XInput support
 *  XInput support requires
 *    https://github.com/dmadison/ArduinoXInput
 *    
 *  USB HID support requires
 *    https://github.com/MHeironimus/ArduinoJoystickLibrary
 *    https://github.com/dmadison/HID_Buttons
 */

// Choose either XInput.h or Joystick.h - not both
//#include <XInput.h>

// Both Joystick.h and HID_Buttons.h are required if you want a standard joystick instead of XInput
#include <Joystick.h>
#include <HID_Buttons.h>

// Uncomment to enable debug printing
//#define DEBUG_PAD

//#define DEBUG_PINS // debug pin readings
//#define DEBUG_DEBOUNCE // debug debounce timers
#define DEBUG_VOLTAGE // debug pin voltages
#define DEBUG_CALIBRATION // debug calibration routine

#define DEBUG_DELAY 1
#define SAMPLES 500

//pin mappings for where things got soldered
const int Pin_Up        = A0;
const int Pin_Right     = A1;
const int Pin_Down      = A2;
const int Pin_Left      = A3;
const int Pin_Back      = A9;
const int Pin_Start     = A8;
const int LED           = 17;
const int p[6] = {Pin_Up, Pin_Right, Pin_Down, Pin_Left, Pin_Start, Pin_Back};

#ifdef JOYSTICK_h
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
  6, 0,                   // Button Count, Hat Switch Count
  false, false, false,    // No X, Y, or Z axes
  false, false, false,    // No Rx, Ry, or Rz
  false, false,           // No rudder or throttle
  false, false, false);   // No accelerator, brake, or steering

JoystickButton HID_Button_Up(0);
JoystickButton HID_Button_Down(1);
JoystickButton HID_Button_Left(2);
JoystickButton HID_Button_Right(3);
JoystickButton HID_Button_Back(4);
JoystickButton HID_Button_Start(5);

#endif

// debounce vars
unsigned long lastDebounceTime[6] = {0};
bool lastState[6] = {false};
const int debounceDelayPad = 5; // debounce time in ms, adjust as needed
const int debounceDelayButton = 10; // debounce time in ms, adjust as needed
long calibrateTimer1 = 0;
long calibrateTimer2 = 0;
long lastCalibration = 0;

//Trigger a button press event if the analog read < (analog read * tt)
//<- 0 sensitivity 1.0 ->
int tt[6] = {0};
const float TriggerPercent = .20;

// state vars
//analog read values
int a[6] = {0};
// Button states
bool s[6] = {0};

void setup() {  
  //The analog pins are configured with internal pull-up resistors, which makes for a very simple circuit
  //However this method does not support useful pressure sensitivity adjustments
  //By soldering 1K resistors as pull-ups on the board, you can make the buttons require more pressure
  //The first version did that, but making the buttons more difficult didn't seem very desirable
  
  pinMode(Pin_Up,         INPUT_PULLUP);
  pinMode(Pin_Right,      INPUT_PULLUP);
  pinMode(Pin_Down,       INPUT_PULLUP);
  pinMode(Pin_Left,       INPUT_PULLUP);
  pinMode(Pin_Start,      INPUT_PULLUP);
  pinMode(Pin_Back,       INPUT_PULLUP);

  pinMode(LED,            OUTPUT);
  
  calibrate();

  #ifdef XINPUT_USB
    XInput.setAutoSend(true); // send presses immediately instead of as a batch
  
    XInput.begin();
  #endif
  
  #ifdef JOYSTICK_h
    Joystick.begin();
  #endif
}

void loop() {    
  //read each pin, and set that Joystick button appropriately
  for(int i = 0; i < 6; ++i)
  {
    a[i] = analogRead(p[i]);
    
    // normalize values to on or off (20 is a fallback; on my system, pad presses usually register around 16)
    if (a[i] <= tt[i] || a[i] < 20) {
      s[i] = true;
    } else {
      s[i] = false; 
    }
    
    switch(p[i]) {
      case Pin_Up:
        if (stateChanged(i, debounceDelayPad)) {
          #ifdef XINPUT_USB
            XInput.setButton(BUTTON_Y, s[i]);
          #endif
          #ifdef JOYSTICK_h
            HID_Button_Up.set(s[i]);
          #endif
        }
        break;
      case Pin_Right:
        if (stateChanged(i, debounceDelayPad)) {
          #ifdef XINPUT_USB
            XInput.setButton(BUTTON_B, s[i]);
          #endif
          #ifdef JOYSTICK_h
            HID_Button_Right.set(s[i]);
          #endif
        }
        break;
      case Pin_Down:
        if (stateChanged(i, debounceDelayPad)) {
          #ifdef XINPUT_USB
            XInput.setButton(BUTTON_A, s[i]);
          #endif
          #ifdef JOYSTICK_h
            HID_Button_Down.set(s[i]);
          #endif
        }
        break;
      case Pin_Left:
        if (stateChanged(i, debounceDelayPad)) {
          #ifdef XINPUT_USB
            XInput.setButton(BUTTON_X, s[i]);
          #endif
          #ifdef JOYSTICK_h
            HID_Button_Left.set(s[i]);
          #endif
        }
        break;
      case Pin_Start:
        if (stateChanged(i, debounceDelayButton)) {
          #ifdef XINPUT_USB
            XInput.setButton(BUTTON_START, s[i]);
          #endif
          #ifdef JOYSTICK_h
            HID_Button_Start.set(s[i]);
          #endif
        }
        break;
      case Pin_Back:
        if (stateChanged(i, debounceDelayButton)) {
          #ifdef XINPUT_USB
            XInput.setButton(BUTTON_BACK, s[i]);
          #endif
          #ifdef JOYSTICK_h
            HID_Button_Back.set(s[i]);
          #endif
        }
        break;
    }    
  }
  
  // Recalibrate pad if BACK and START are pressed and held together for 10 seconds
  if (a[4] < tt[4] && lastState[4] == false) {
    #if !defined (USB_XINPUT) && defined (DEBUG_PAD) && defined (DEBUG_CALIBRATION)
      Serial.println("Resetting calibration timer 1");
    #endif
    calibrateTimer1 = millis();
  }
  if (a[5] < tt[5] && lastState[5] == false) {
    #if !defined (USB_XINPUT) && defined (DEBUG_PAD) && defined (DEBUG_CALIBRATION)
      Serial.println("Resetting calibration timer 2");
    #endif
    calibrateTimer2 = millis();
  }
 
  if (a[4] >= tt[4] && a[5] >= tt[5] && (lastState[4] == true || lastState[5] == true) && millis() - calibrateTimer1 >= 10000 && millis() - calibrateTimer2 >= 10000) {
    #if !defined (USB_XINPUT) && defined (DEBUG_PAD) && defined (DEBUG_CALIBRATION)
      char buffer[80];
      sprintf(buffer, "%lu %lu %lu %lu %d %d %d %d", lastCalibration, calibrateTimer1, calibrateTimer2, millis(), s[4], lastState[4], s[5], lastState[5]);
      Serial.println(buffer);
      Serial.println("Calibrating");
    #endif
    calibrateTimer1 = millis();
    calibrateTimer2 = millis();
    delay(1000); // give time to step off the buttons before starting
    calibrate();
  }          

  for (int i = 0; i < 6; ++i) {
    lastState[i] = s[i];    
  }
  
  #if !defined (USB_XINPUT) && defined (DEBUG_PAD)
    char buffer[80];
    #ifdef DEBUG_PINS
      // Analog read values
      sprintf(buffer, "Pins: %d(%d) %d(%d) %d(%d) %d(%d) (%d %d)", a[0], tt[0], a[1], tt[1], a[2], tt[2], a[3], tt[3], a[4], a[5] );
      Serial.println(buffer);
    #endif
    #ifdef DEBUG_VOLTAGE
      printVoltage();
    #endif
    #ifdef DEBUG_DEBOUNCE
      sprintf(buffer, "Debounce: %lu %lu %lu %lu (%lu %lu) millis(): %lu", lastDebounceTime[0], lastDebounceTime[1], lastDebounceTime[2], lastDebounceTime[3], lastDebounceTime[4], lastDebounceTime[5], millis());
      Serial.println(buffer);
    #endif
    delay(DEBUG_DELAY);
  #endif
}

//returns true if a button state has changed
bool stateChanged(int buttonIndex, int debounceDelay) {
  // always return true if no delay
  if (debounceDelay == 0) {
    return true;
  }
  
  // If the switch changed, due to noise or pressing:
  if (s[buttonIndex] != lastState[buttonIndex]) {
    // reset debounce timer
    lastDebounceTime[buttonIndex] = millis();
  }
    
  if ((millis() - lastDebounceTime[buttonIndex]) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (s[buttonIndex] != lastState[buttonIndex]) {
      lastState[buttonIndex] = s[buttonIndex];
    }
    return true;
  }
  return false;
}

void calibrate() {
  Serial.println("Calibrating pad thresholds");
  // Calibrate trigger thresholds per button
  // If something was sitting on a button when you plugged your controller in or reset it, shame on you... replug or reset to recalibrate
  int low[6] = {0};
  int hi[6] = {0};

  for (int t = 0; t < SAMPLES; ++t) {
    digitalWrite(LED, LOW);
    for (int i = 0; i < 6; ++i) {
      int val = analogRead(p[i]);

      // initialize low and high values
      if (low[i] == 0) { low[i] = val; }
      if (hi[i] == 0) { hi[i] = val; }

      // set high and low values
      if (val < hi[i] && val > 0) { low[i] = val; } 
      if (val > hi[i] && val > 0) { hi[i] = val; }

      #if !defined (USB_XINPUT) && defined (DEBUG_PAD) && defined (DEBUG_CALIBRATION)
        char buffer[80];
        sprintf(buffer, "pass %d: idx: %d lo/hi: %d/%d", t, i, low[i], hi[i]);
        Serial.println(buffer);
        delay(1);
      #endif
      // reset the calibration if there's too big of a gap between high and low values
      if (hi[i] - low[i] >= 30) { 
        memset(low, 0, sizeof(low));
        memset(hi, 0, sizeof(low));
        t = 0; 
        i = 0; 
      } 
    }
    digitalWrite(LED, HIGH);
  }

  for (int i = 0; i < 7; ++i) {
    tt[i] = low[i] * TriggerPercent;
  }
  #if !defined (USB_XINPUT) && defined (DEBUG_PAD) && defined (DEBUG_CALIBRATION)
    char buffer[80];
    // Analog read values
    sprintf(buffer, "Thresh/Lo/Hi: %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d (%d/%d/%d %d/%d/%d)", tt[0], low[0], hi[0], tt[1], low[1], hi[1], tt[2], low[2], hi[2], tt[3], low[3], hi[3], tt[4], low[4], hi[4], tt[5], low[5], hi[5]);
    Serial.println(buffer);
  #endif
  lastCalibration = millis();
  return;
}

#ifdef DEBUG_VOLTAGE
void printVoltage() {
  for (int i = 0; i < 6; ++i) {
    String temp = String(a[i] * (5.0 / 1023.0), 2);
    switch (p[i]) {
      case Pin_Up:
        Serial.print("vU:");
      break;
      case Pin_Down:
        Serial.print("vD:");
      break;
      case Pin_Right:
        Serial.print("vR:");
      break;
      case Pin_Left:
        Serial.print("vL:");
      break;
      case Pin_Back:
        Serial.print("vB:");
      break;
      case Pin_Start:
        Serial.print("vS:");
      break;
    }
    Serial.print(temp);
    Serial.print(" ");
  }
  Serial.println();
}
#endif
