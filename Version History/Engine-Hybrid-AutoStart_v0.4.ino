// Engine-Hybrid-AutoStart v0.4
// Writter: Chance Johnson
// Youtube Channel: Mechanceism
// Libraries are not by me, additionally only the uncommented libraries are required currently
// Written using Arduino IDE 2.3.4
// Libraries can be found in library manager
// Use or modify as you like just leave some credit to the original
// Good Luck! There are bugs and they will bite

#include <Servo.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>
#include "InputDebounce.h"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40); //default is 0x40

// Constants for easy adjustment
const int holdMode = 1;            // 0 = momentary button Starter, 1 = Timed latching Starter, Both with RPM cutoff
bool isLimiter = false;            // RPM Rev Limiter: 0 = Disabled, 1 = Enabled
bool isAutoRestart = false;        // AutoRestart after engine dies
bool isGearIndicator = true;       // If gear lights and neutral safety is used
const int limiterTime = 250;       // Time (ms) RelayKill stays HIGH when RPM limit is exceeded
const int rpmKillThreshold = 100;  // Lower RPM threshold to reset RelayKill
const int rpmRunThreshold = 2000;  // RPM limit to be considered started
const int rpmMaxThreshold = 8000;  // RPM limit for limiter
const int OptempThreshold = 25;    // Temperature threshold for choke control
const int OvertempThreshold = 100; // Temperature threshold for choke control
const int ledBlinkTime = 500;      // Time (ms) for LED blink when no gear selected
const int starterOvertime = 2000;  // How long between start button turns to Kill after starting, holdMode = 0 only, 1 is instant
const int starterTimeout = 5000;   // Maximum time (ms) for the starter relay to stay HIGH, holdMode = 1 only
const int numPotReadings = 1;      // Number of readings for potentiometer smoothing, 1 = No Smoothing
const int BUTTON_DEBOUNCE_DELAY = 100;   // [ms] Delay time between button presses
bool InvertRelayStarter = true;    // Do you want to invert Starter Relay Output logic?
bool InvertRelayKill = true;       // Do you want to invert Kill Relay Output logic?

// Pinouts
const int RelayStarter = 3;
const int RelayKill = 4;
const int Button1Pin = 5;
const int Button2Pin = 6;
const int DriveLedPin = 7;
const int NeutralLedPin = 8;
const int ReverseLedPin = 9;
const int DrivePin = 10;
const int NeutralPin = 11;
const int ReversePin = 12;
//const int tempSensorPin = 12;
const int CoilPulsePin = 2;
const int PotentiometerThrottle = A1;
const int AnalogTempPin = A6;
const int throttlePin = 0; //not arduino pin #, servo breakout location
const int chokePin = 1; //not arduino pin #, servo breakout location
// Servo pins are defined in setup for direct connection from arduino

// Servo min/max positions
const int throttleMin = 425; // servo pulse length (out of 4096) for throttle closed
const int throttleMax = 340; // servo pulse length (out of 4096) for throttle open
const int chokeMin = 400; // servo pulse length (out of 4096) for choke open
const int chokeMax = 285; // servo pulse length (out of 4096) for choke closed
const int SERVO_FREQ = 50; // Analog servos run at ~50 Hz updates
int throttleValue = throttleMin;
int chokeValue = chokeMin;
int chokePercent = 0;

//Servo Throttle;
//Servo Choke;

// OneWire & Temperature Sensor Setup
//OneWire oneWire(tempSensorPin);
//DallasTemperature sensors(&oneWire);

// RPM Counter
//const int Coilthreshold = 512; // Adjust threshold for detecting pulses
volatile int CoilPulseCount = 0;
volatile int CoilPulsesPerMinute = 0;
bool rpmMaxThresholdState = false;
bool rpmRunThresholdState = false;
bool rpmKillThresholdState = false;
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
int throttleAverage = 0;

// Gear Selection
int lastActiveGear = -1; // 0 = Drive, 1 = Neutral, 2 = Reverse
bool DriveState = false;
bool NeutralState = false;
bool ReverseState = false;
bool AllowStartButton = false;

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
  attachInterrupt(digitalPinToInterrupt(CoilPulsePin), CoilPulseISR, FALLING);

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

  //Inver Starter and Kill Logic
  if(InvertRelayStarter == false) {
    ISHIGH = LOW;
    ISLOW = HIGH;
  }
  else {
    ISHIGH = HIGH;
    ISLOW = LOW;
  }

  if(InvertRelayKill == false) {
    IKHIGH = LOW;
    IKLOW = HIGH;
  }
  else {
    IKHIGH = HIGH;
    IKLOW = LOW;
  }
  digitalWrite(RelayStarter, ISHIGH); // Relays are active LOW
  digitalWrite(RelayKill, IKHIGH); // Relays are active LOW
  
  Serial.begin(9600);
  //sensors.begin(); // for onewire temp

  // Servos
  //Throttle.attach(A2);
  //Choke.attach(A3);
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);

  // Buttons
  pinMode(pinLED, OUTPUT); 
  // register callback functions (shared, used by all buttons)
  Button1.registerCallbacks(buttonTest_pressedCallback, buttonTest_releasedCallback); //, buttonTest_pressedDurationCallback, buttonTest_releasedDurationCallback);
  Button2.registerCallbacks(buttonTest_pressedCallback, buttonTest_releasedCallback);
// setup input buttons (debounced)
  Button1.setup(Button1Pin, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);
  Button2.setup(Button2Pin, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES); // ,500); single-shot pressed-on time duration callback
  
}
//-----------------------------------------------------------------------------------------------------
void loop() {
  unsigned long now = millis();
  Button1.process(now); // callbacks called in context of this function
  Button2.process(now);

  //ReadTemp();
  //Temp();
  //RPMCounter();
  UpdateRPM();

  if(isGearIndicator == true){
    Gear();
  } else {
    AllowStartButton = true;
  }

  Potentiometer();
  Servos();
  if(isLimiter == true) {
    Limiter();
  }
  Relay();

  
  //TestRestart();
  //Serial.print("throttleValue: ");
  //Serial.print(throttleValue);
  //Serial.print("    chokeValue: ");
  //Serial.println(chokeValue);


}
//-----------------------------------------------------------------------------------------------------

void CoilPulseISR() { // Interrupt Service Routine, used for counting pulse immediatly, Code is redirected to this
  CoilPulseCount++;
}

void UpdateRPM() {
  static unsigned long lastRPMCheck = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastRPMCheck >= 200) {  // Update every 100ms
    lastRPMCheck = currentMillis;

    noInterrupts();
    int pulseCount = CoilPulseCount; // Copy safely
    CoilPulseCount = 0; // Reset counter
    interrupts();

    if (pulseCount > 0) {
      CoilPulsesPerMinute = (pulseCount * 60000.0) / 200; // Convert to PPM
    } else {
      CoilPulsesPerMinute = 0; // Avoid stale values
    }

    Serial.print("Pulses per minute: ");
    Serial.println(CoilPulsesPerMinute);

    // Threshold logic
    rpmMaxThresholdState = CoilPulsesPerMinute >= rpmMaxThreshold;
    rpmRunThresholdState = CoilPulsesPerMinute >= rpmRunThreshold;
    rpmKillThresholdState = CoilPulsesPerMinute >= rpmKillThreshold;

    if (!rpmRunThresholdState) {
      LastRunningtime = millis();
    }
  }
}

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

void ReadTemp() {
  //sensors.requestTemperatures();
  //float tempC = sensors.getTempCByIndex(0);
  //Serial.print("Temperature: ");
  //Serial.print(tempC);
  //Serial.println("°C");
}

void Servos() {
  throttleValue = map(throttleAverage, 0, 1023, throttleMin, throttleMax);
  chokeValue = map(chokePercent, 0, 100, chokeMin, chokeMax);

  if(Shutoff == true || OverTemp == true) {
    //Throttle.write(throttleMin);
    //Choke.write(chokeMax);
    pwm.setPWM(throttlePin, 0, throttleMin);
    pwm.setPWM(chokePin, 0, chokeMax);
  }
  else {
    //Throttle.write(throttleValue);
    pwm.setPWM(throttlePin, 0, throttleValue);

    if(OpTemp == false) {
      //Choke.write(chokeValue);
      pwm.setPWM(chokePin, 0, chokeValue);
    }
    else {
      //Choke.write(chokeMin);
      pwm.setPWM(chokePin, 0, chokeMin);
    }
  }
  
}

void Potentiometer() {
  // Read and smooth potentiometer input
  total -= readings[readIndex];
  readings[readIndex] = analogRead(PotentiometerThrottle);
  total += readings[readIndex];
  readIndex = (readIndex + 1) % numPotReadings;
  throttleAverage = total / numPotReadings;
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
  if (isLimiter == true && rpmMaxThresholdState == true && limiterStartTime == 0) {
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

  if(pinIn == Button1Pin && Button1State == 0) {
    Button1State = 1;
    Serial.println("Button1State == 1");
  }
  //else if (pinIn == Button1Pin && Button1State == 1) {
    //int Button1State = 0;
    //Serial.println("Button1State == 0");
  //}
  if(pinIn == Button2Pin && Button2State == 0) {
    Button2State = 1;
    Serial.println("Button2State == 1");
  }
  else if (pinIn == Button2Pin && Button2State == 1) {
    Button2State = 0;
    Serial.println("Button2State == 0");
  }
}

void buttonTest_releasedCallback(uint8_t pinOut) {
  digitalWrite(pinLED, LOW); // turn the LED off
  //Serial.println(pinOut); 

  if (pinOut == Button1Pin && Button1State == 1) {
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
