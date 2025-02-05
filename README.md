# Engine-Hybrid-AutoStart
Adding basic control features to small engines

This is the first release and I'm sure many things are not working perfectly or written well

Most of the code structure is there, it's just getting to connecting them together to make things work

# Designed for Microcontrollers, Simple Carbuerated Engines / Electric Motors, and Hobbyists
## Current supported devices
 - 5V logic boards
   - Arduino nano/mega/uno/etc.
 - 3.3V logic Boards if you change the voltage dividor resistor values
 - 2 relays/mosffets
 - 1 temp sensor
 - 1 DAC (Digital to Analog Converter)
 - 2 servo motors
 - 1 Spark coil (and a voltage divider with octocoupler)

## Custom Parts
 - Dual servo (Throttle & Choke) mounting bracket for a pz20 carbuerator
 - Uses MG996R 55g Metal Gear Torque Digital Servo
 - (Future) EFI Throttle body and injector mount

## Current Test Engines: 
 - knock off Tao Tao 125cc 3 speed atv Carbuerated
 - 3kw electric motor

## For More Information
 - Youtube Channel: https://www.youtube.com/@mechanceism
 - Playlist: https://www.youtube.com/playlist?list=PLbks487P7yJCsIAiZIUsBMxj5zc2dP7JO
 - Basic rewiring: https://youtu.be/ws6BDvNJcOI
 - Hardwire Hybrid Inital setup: https://youtu.be/ayrcGL1MsC4
 - Digitized Carbuerator: https://youtu.be/1UslfvnQJVw

# Features
 - RPM monitering (based off spark signal)
   - Simple resistor octocoupler paired with a voltage divider
   - Parallel to the spark coil
 - Digitized Carbortor
   - PWM throttle and choke control
     - Potentiometer passthrough
     - temperture adjusting choke
 - Analog Throttle output
   -  For secondary controller (Gas / Electric / Hybrid)
   -  Biased throttle sharing 
 - Invertable relay/mosffet Starter & Kill Switch control
 - Dual Input Mode starter motor control (debounced)
   - Momentary passthrough with RPM cutoff
   - Toggled timed attempt with RPM cutoff
 - AutoRestart
   - If the engine turns off it will attempt to restart (if enabled)
 - RPM Rev Limiter (if enabled)
   - Time based adjustable
 - Gear Selection Indicator
   - Shows current gear
   - Flashes last gear if gear change jammed
   - Starter motor lockout if not in neutral
     - CVT override
 - Engine Temperature measurments
   - Resistive (analoge input)
   - digital (if I can get onewire to read DS18B20 faster)
     - Under Temp
     - Operating temp
     - Over Temp
 - Safe Shutdown
   - Overtemp shudown
   - Emergency Shutdwon

# Plans (Not in order)
 - Flywheel based timing
 - CDI box replacement Platform (Currently still required)
 - Basic EFI support
 - Advance and delay timing
 - Constant RPM throttle Compensation Option (set and potentiometer based) (Corrently throttle is degree open based)
 - Auto idle Compensation
 - Auto shifting support
 - Support for abs speed sensors
 - Constant Speed throttle Compensation Option (set and potentiometer based) (Corrently throttle is degree open based)
 - Remote start/shutdown
  - Dual directional transmission key fobs
 - Servo Breakout Board Support
 - Support more thermocouples
 - Analog and PWM throttle output Support
 - PWM Throttle Support
 - dual throttle Checking Support
 - Startup Rev Option (Not designed for CVTs)
 - Source Voltage detection (For Generators power appliances or charging batteries)
  - Outputs for disabling/enabling main breaker and generator breaker
 - Fuel level estimation/warning/fuel low shutdown
 - Fuel priming
 - Fuel Cutoff
