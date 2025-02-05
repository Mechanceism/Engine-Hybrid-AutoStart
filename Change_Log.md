# Version History:

## 0.0
 - Initial concepts
 - Basic rewiring: https://youtu.be/ws6BDvNJcOI
 - Hardwire Hybrid Inital setup: https://youtu.be/ayrcGL1MsC4
 - Digitized Carbuerator: https://youtu.be/1UslfvnQJVw

## 0.1
 - Initial debounce code (later removed)

## 0.2
 - Additing initial logic in series
 - Changed debounce to use inputdebounce library
 - Added onewire and dallastemperture
 - Added gear indicator 
 - Added most features but I started over to change logic from series to function based

## 0.3
 - Started over using functions
 - 3 days of 12 hour testing
 - Created repository
 - Commented out onewire and dallastemperture due to onoewire delay
 - Replaced temp with analog thermocouple
 - RPM monitering (based off spark signal, sorry not flywheel based, for wider support/simplicity)
   - Simple resistor octocoupler paired with a voltage divider
   - Parallel to the spark coil
 - Digitized Carbortor
   - Added servo mounting bracket for MG996R 55g Metal Gear Torque Digital Servo
   - PWM throttle and choke control
     - Potentiometer passthrough
     - temperture adjusting choke
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
