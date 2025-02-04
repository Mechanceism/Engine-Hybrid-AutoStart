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
 - (Future) EFI Throttle body and injector mount

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
