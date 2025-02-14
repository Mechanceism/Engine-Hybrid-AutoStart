// Engine-Hybrid-AutoStart v0.3
// Writter: Chance Johnson
// Youtube Channel: Mechanceism
// Libraries not by me, only the uncommented libraries are required currently
// Written using Arduino IDE 2.3.4
// Libraries can be found in library manager
// Use or modify as you like just leave some credit to the original
// Good Luck! There are bugs and they will bite

#include <Servo.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>
#include "InputDebounce.h"

// Constants for easy adjustment
const int holdMode = 1;            // 0 = momentary button Starter, 1 = Timed latching Starter, Both with RPM cutoff
const int isLimiter = 0;           // RPM Rev Limiter: 0 = Disabled, 1 = Enabled
const int isAutoRestart = 1;       // AutoRestart after engine dies
const int limiterTime = 250;       // Time (ms) RelayKill stays HIGH when RPM limit is exceeded
const int rpmKillThreshold = 100;  // Lower RPM threshold to reset RelayKill
const int rpmRunThreshold = 1000;   // RPM limit to be considered started
const int rpmMaxThreshold = 8000;  // RPM limit for limiter
const int OptempThreshold = 25;      // Temperature threshold for choke control
const int OvertempThreshold = 100;      // Temperature threshold for choke control
const int ledBlinkTime = 500;      // Time (ms) for LED blink when no gear selected
const int starterOvertime = 2000;  // How long between start button turns to Kill after starting, holdMode = 0 only, 1 is instant
const int starterTimeout = 5000;   // Maximum time (ms) for the starter relay to stay HIGH, holdMode = 1 only
const int numPotReadings = 1;      // Number of readings for potentiometer smoothing, 1 = No Smoothing
#define BUTTON_DEBOUNCE_DELAY   100   // [ms]
bool InvertRelayStarter = true;
bool InvertRelayKill = true;

// Pinouts
const int RelayStarter = 2;
const int RelayKill = 3;
const int Button1Pin = 4;
const int Button2Pin = 5;
const int DriveLedPin = 6;
const int NeutralLedPin = 7;
const int ReverseLedPin = 8;
const int DrivePin = 9;
const int NeutralPin = 10;
const int ReversePin = 11;
const int tempSensorPin = 12;
const int CoilPulsePin = A3;
const int PotentiometerThrottle = A4;
const int AnalogTempPin = A7;
// Servo pins defined in setup


// OneWire & Temperature Sensor Setup
//OneWire oneWire(tempSensorPin);
//DallasTemperature sensors(&oneWire);


// RPM Counter
const int Coilthreshold = 512; // Adjust threshold for detecting pulses
volatile int CoilPulseCount = 0;
unsigned long lastCoilMillis = 0; 
unsigned long CurrentCoilMillis = 0;
volatile int CoilPulsesPerMinute = 0;
static bool lastCoilState = false;
int CoilPulseState = HIGH;
bool rpmMaxThresholdState = 0;
bool rpmRunThresholdState = 0;
bool rpmKillThresholdState = 0;
unsigned long LastRunningtime = millis();

//Buttons
static const int pinLED = LED_BUILTIN; // 13
static InputDebounce Button1; // not enabled yet, setup has to be called first, see setup() below
static InputDebounce Button2;
int Button1State = 0;
int Button2State = 0;
int pinIn = -1;
int pinOut = -1;

//Potentiometer
int readings[numPotReadings];
int readIndex = 0;
int total = 0;
int average = 0;

// Servo min/max positions
const int throttleMin = 170;
const int throttleMax = 120;
const int chokeMin = 0;
const int chokeMax = 60;
int throttleValue = throttleMin;
int chokeValue = chokeMin;
Servo Throttle;
Servo Choke;

// Gear Selection
int lastActiveGear = -1; // 0 = Drive, 1 = Neutral, 2 = Reverse
bool DriveState = !digitalRead(DrivePin);
bool NeutralState = !digitalRead(NeutralPin);
bool ReverseState = !digitalRead(ReversePin);
bool AllowStartButton = NeutralState && !DriveState && !ReverseState;

//Limiter
unsigned long limiterStartTime = 0;
bool limiterActive = false;

//Relay
bool RelayStarterState = false; // Current state of the output (false = off, true = on)
unsigned long lastToggleTime = 0; // Time when the output was last toggled on
int lastButton1State = 0;
bool timerActive = false;
bool Shutoff = false;

//Restart
int RiseState = 0;
bool TriggerRestart = false;

//Temperature
int AnalogTemp = 0;
int TempRead = 0;
bool OpTemp = false;
bool OverTemp = false;

//Invert
bool ISHIGH = HIGH;
bool ISLOW = LOW;
bool IKHIGH = HIGH;
bool IKLOW = LOW;

//---------------------------------------------------------------------------------------------------------------
void setup() {
  // Pin Modes
  pinMode(Button1Pin, INPUT_PULLUP);
  pinMode(Button2Pin, INPUT_PULLUP);
  pinMode(CoilPulsePin, INPUT);

  pinMode(RelayStarter, OUTPUT);
  pinMode(RelayKill, OUTPUT);

  pinMode(DrivePin, INPUT_PULLUP);
  pinMode(NeutralPin, INPUT_PULLUP);
  pinMode(ReversePin, INPUT_PULLUP);
  pinMode(DriveLedPin, OUTPUT);
  pinMode(NeutralLedPin, OUTPUT);
  pinMode(ReverseLedPin, OUTPUT);

  pinMode(AnalogTemp, INPUT);
  pinMode(PotentiometerThrottle, INPUT);
  
  Serial.begin(9600);
  //sensors.begin(); // for onewire temp

  // Servos
  Throttle.attach(A5);
  Choke.attach(A6);

  // Button Testing
  pinMode(pinLED, OUTPUT); 
  // register callback functions (shared, used by all buttons)
  Button1.registerCallbacks(buttonTest_pressedCallback, buttonTest_releasedCallback); //, buttonTest_pressedDurationCallback, buttonTest_releasedDurationCallback);
  Button2.registerCallbacks(buttonTest_pressedCallback, buttonTest_releasedCallback);
  //buttonTestC.registerCallbacks(buttonTest_pressedCallback, buttonTest_releasedCallback, buttonTest_pressedDurationCallback, buttonTest_releasedDurationCallback);
  // setup input buttons (debounced)
  Button1.setup(Button1Pin, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);
  Button2.setup(Button2Pin, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES); // ,500); single-shot pressed-on time duration callback

  if(InvertRelayStarter == false) {
    ISHIGH = HIGH;
    ISLOW = LOW;
  }
  else {
    ISHIGH = LOW;
    ISLOW = HIGH;
  }

  if(InvertRelayKill == false) {
    IKHIGH = HIGH;
    IKLOW = LOW;
  }
  else {
    IKHIGH = LOW;
    IKLOW = HIGH;
  }
  digitalWrite(RelayStarter, ISHIGH); // Relays are active LOW
  digitalWrite(RelayKill, IKHIGH); // Relays are active LOW
  
}
//-----------------------------------------------------------------------------------------------------
void loop() {
  unsigned long now = millis();
  Button1.process(now); // callbacks called in context of this function
  Button2.process(now);

  ReadTemp();
  RPMCounter();
  Gear();
  Potentiometer();
  Servos();
  Limiter();
  Relay();

  
  //TestRestart();
  //Serial.println(average);


}
//-----------------------------------------------------------------------------------------------------

void TestRestart() {
 if(millis() >= 10000 && RiseState == 0) {
    RiseState = 1;
    TriggerRestart = true;
 }
}

void Temp() {
  AnalogTemp = analogRead(AnalogTempPin);
  TempRead = map(AnalogTemp, 0, 1023, -127, 127);
  if(OvertempThreshold >= TempRead >= OptempThreshold) {
    OpTemp = true;
    OverTemp = false;

  }
  else if (TempRead >= OvertempThreshold) {
    OpTemp = false;
    OverTemp = true;
  }
}

void Servos() {
  if(Shutoff == true || OverTemp == true) {
    Throttle.write(throttleMin);
    Choke.write(chokeMin);
  }
  else {
    throttleValue = map(average, 0, 1023, throttleMin, throttleMax);
    Throttle.write(throttleValue);

    if(OpTemp == true) {
      Choke.write(chokeMax);
    }
    else {
      Choke.write(chokeMin);
    }
  }
  
}

void RPMCounter() {
  bool currentCoilState = digitalRead(CoilPulsePin);
  if (!currentCoilState && lastCoilState) {  // Detect falling edge (HIGH to LOW)
    CoilPulseCount++;
    unsigned long currentMillis = millis();  // Get current time

      // If this is not the first pulse, calculate elapsed time
    if (lastCoilMillis > 0) {  
      float elapsedCoilTime = currentMillis - lastCoilMillis;
      CoilPulsesPerMinute = 60000.0 / elapsedCoilTime;
      Serial.print("Time between pulses: ");
      Serial.println(elapsedCoilTime);
      Serial.print("Pulses per minute: ");
      Serial.println(CoilPulsesPerMinute);
    }
    lastCoilMillis = currentMillis;  // Update last recorded time
  }
  lastCoilState = currentCoilState;  // Store last state
  
  if(CoilPulsesPerMinute >= rpmMaxThreshold) {
    rpmMaxThresholdState = 1;
  }
  else {
    rpmMaxThresholdState = 0;
  }

  if(CoilPulsesPerMinute >= rpmRunThreshold) {
    rpmRunThresholdState = 1;
  }
  else {
    rpmRunThresholdState = 0;
    LastRunningtime = millis();
  }

  if(CoilPulsesPerMinute >= rpmKillThreshold) {
    rpmKillThresholdState = 1;
  }
  else {
    rpmKillThresholdState = 0;
  }
}

void ReadTemp() {
  //sensors.requestTemperatures();
  //float tempC = sensors.getTempCByIndex(0);
  //Serial.print("Temperature: ");
  //Serial.print(tempC);
  //Serial.println("°C");
}

void Potentiometer() {
  // Read and smooth potentiometer input
  total -= readings[readIndex];
  readings[readIndex] = analogRead(PotentiometerThrottle);
  total += readings[readIndex];
  readIndex = (readIndex + 1) % numPotReadings;
  average = total / numPotReadings;
}

void Gear() {
  unsigned long now = millis();
  DriveState = !digitalRead(DrivePin);
  NeutralState = !digitalRead(NeutralPin);
  ReverseState = !digitalRead(ReversePin);

  AllowStartButton = NeutralState && !DriveState && !ReverseState;

  if (DriveState) {
    digitalWrite(DriveLedPin, HIGH);
    digitalWrite(NeutralLedPin, LOW);
    digitalWrite(ReverseLedPin, LOW);
    lastActiveGear = 0;
  } else if (NeutralState) {
    digitalWrite(DriveLedPin, LOW);
    digitalWrite(NeutralLedPin, HIGH);
    digitalWrite(ReverseLedPin, LOW);
    lastActiveGear = 1;
  } else if (ReverseState) {
    digitalWrite(DriveLedPin, LOW);
    digitalWrite(NeutralLedPin, LOW);
    digitalWrite(ReverseLedPin, HIGH);
    lastActiveGear = 2;
  } else {
    if (lastActiveGear == 0) {
      digitalWrite(DriveLedPin, (now / ledBlinkTime) % 2);
    } else if (lastActiveGear == 1) {
      digitalWrite(NeutralLedPin, (now / ledBlinkTime) % 2);
    } else if (lastActiveGear == 2) {
      digitalWrite(ReverseLedPin, (now / ledBlinkTime) % 2);
    }
    AllowStartButton = false;
  }
}

void Relay() {
  switch(holdMode) {
    case 0: 
      if(CoilPulsesPerMinute <= rpmRunThreshold) {
        if(Button1State == 1 && AllowStartButton == true ) {
          digitalWrite(RelayStarter, ISLOW);
        }
        else {
          digitalWrite(RelayStarter, ISHIGH);
        }
      }
      else if (CoilPulsesPerMinute >= rpmRunThreshold && LastRunningtime - millis() >= starterOvertime) {
        if(Button1State == 1) {
          digitalWrite(RelayKill, IKLOW);
        }
        else {
          digitalWrite(RelayKill, IKHIGH);
        }
      }
      break;
    
    case 1: 
      if (Button1State == 1 && lastButton1State == 0) { 
        RelayStarterState = true;  // Turn ON output
        digitalWrite(RelayStarter, ISLOW); // Activate relay
      }
      if (Button1State == 0 && lastButton1State == 1) { // Button 1: Falling edge detection to start timer
        if (RelayStarterState) {  
          lastToggleTime = millis();  // Start timer
          timerActive = true;
        }
      }
      if (Button1State == 1 && lastButton1State == 0) { // Button 1: Rising edge detection while timer is running should cancel the timer & turn OFF
        if (RelayStarterState && timerActive) { 
          RelayStarterState = false;  // Turn OFF output
          digitalWrite(RelayStarter, ISHIGH); // Deactivate relay
          timerActive = false;  // Stop the timer
        }
      }
      lastButton1State = Button1State; // Update button state tracking
      if (RelayStarterState && timerActive && millis() - lastToggleTime >= starterTimeout) { // Timeout logic: turn OFF after duration
        RelayStarterState = false;
        digitalWrite(RelayStarter, ISHIGH);
        timerActive = false;
      }
      if (Button2State == 1 || CoilPulsesPerMinute >= rpmRunThreshold || !AllowStartButton) { // Force OFF if Button2State is pressed OR RPM threshold is met
        RelayStarterState = false;
        digitalWrite(RelayStarter, ISHIGH);
        timerActive = false;
      }
      if (TriggerRestart) { 
        RelayStarterState = true;  // Turn ON output
        digitalWrite(RelayStarter, ISLOW); // Activate relay
        lastToggleTime = millis();  // Start timer
        timerActive = true;
        TriggerRestart = false;  // Reset TriggerRestart so it only runs once per call
      }
    break;
  }

  if (Button2State == 1) {
    digitalWrite(RelayKill, IKLOW);
    AllowStartButton = false;
    Shutoff = true;
  }
  else {
    digitalWrite(RelayKill, IKHIGH);
    AllowStartButton = true;
    Shutoff = false;
  }
}

void Limiter() {
  if (isLimiter == 1 && rpmMaxThresholdState == 1 && limiterStartTime == 0) {
    limiterActive = true;
    limiterStartTime = millis();
    digitalWrite(RelayKill, IKLOW);
  }

  if (limiterActive == true && millis() - limiterStartTime >= limiterTime) {
    digitalWrite(RelayKill, IKHIGH);
    limiterActive = false;
    limiterStartTime = 0;
  }
}

void buttonTest_pressedCallback(uint8_t pinIn) {
  digitalWrite(pinLED, HIGH); // turn the LED on
  //Serial.println(pinIn);

  if(pinIn == 4 && Button1State == 0) {
    Button1State = 1;
    Serial.println("Button1State == 1");
  }
  //else if (pinIn == 4 && Button1State == 1) {
    //int Button1State = 0;
    //Serial.println("Button1State == 0");
  //}
  if(pinIn == 5 && Button2State == 0) {
    Button2State = 1;
    Serial.println("Button2State == 1");
  }
  else if (pinIn == 5 && Button2State == 1) {
    Button2State = 0;
    Serial.println("Button2State == 0");
  }
}

void buttonTest_releasedCallback(uint8_t pinOut) {
  digitalWrite(pinLED, LOW); // turn the LED off
  //Serial.println(pinOut); 

  if (pinOut == 4 && Button1State == 1) {
    Button1State = 0;
    Serial.println("Button1State == 0");
  }
}

void buttonTest_pressedDurationCallback(uint8_t pinIn, unsigned long duration) {
  Serial.print(pinIn);
  Serial.println(duration);
}

void buttonTest_releasedDurationCallback(uint8_t pinIn, unsigned long duration) {
  Serial.print(pinIn);
  Serial.println(duration);
}
