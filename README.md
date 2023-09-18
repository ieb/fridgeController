#  Fridge Controller

This is a controller for 12V Danfoss and Secomp fridges. The compressors on these fridges are
variable speed and typically have switch based external temperature sensors. In a low power
environment running off 12v there are a few problems, which can be addressed by buying one
of the top end control heads. Some of documentation for the newer compressors indicate that
the RPM and minimum battery voltages have more control, but the BD35 and BD50 compressors
while being variable speed have unsuitable battery shutoff voltages.

# Why ?

I've gone through 2x 300 Lead Acid battery sets due to the behavior of the Fridge Compressor, in the past 6 years. Perhaps 3 years is ok for flooded or AGM, but there have been perhaps 60 charge cycles per year, so 180 in total and the batteries have always been kept fully charged. Could be bad luck buying batteries claiming not to be what they are.... but they were not the cheapest and weighed almost the same as Rolls equivalents. (ie same volume of lead).

# Fridge hardware

The compressor (BD35) is a variable speed motor that runs at between 2000 and 3500 RPM. In basic setups
the speed is controlled current from one of the pins on the control unit of between 2mA (3500rpm) and 5mA (2000rpm).
Typically manufacturers add a fixed resistor to achieve this, loosing the benefits of variable speed control,
which allow these compressors to use significantly less energy than the fixed speed counterparts.

The controllers have a battery low voltage cutout which is set by another resistor. Typically this defaults to 10.7v but
can be lowered to 9.6v for solar only applications or 11.7v for other applications. All these voltages are sufficiently
low enough to damage most lead acid batteries, and certainly well below the manufacturer recommended minimum discharge
level of 12.2v. The saving grace is that many fridge cables will be long and there might be enough voltage drop to
disable the fridge before the damage is done (repeatedly). Second saving grace is after the damage is done the battery is
unlikely to maintain is voltage while the fridge is on and should drop to below these levels, switching the fridge off and
then recovering. Typically this is accompanied with low voltage alarms on other systems supplied by the same battery.

The control units for the compressor have a electronic CCU built in. It has a LIN interface (19200 baud 8-n-1 serial),
with the same electrical characteristics found on many K-LINE car interfaces in cars. Cheap pass-through K-LINE OBDII
adapters will almost certainly work. The protocol is similar to Modbus and allows access to almost all parameters, 
however the documentation on the format of the parameters is very sparse. There is a free windows utility xtool4cool, from
secop that allows access to the parameters, but, TBH, this interface is intended for manufacturing and is not worth the
effort to reverse engineer (imho).

The restive compressor control mentioned above can also be controlled by a 5KHz PWM signal driving an open collector NPN
transistor, the collector connected to the control pin. There are no specifications available on the duty cycle but I 
am going to assume the aim is to sink between 2mA and 5mA.

# This hardware

I thought about a Attiny3224, Atmega328p (Pro Mini) or ESP32, all are possible, but I dont want to have a screen as it 
draws too much power and needs buttons, and cant be hidden. Setup will be over serial. Attiny3224 and Atmega328p need a
serial adapter which is why the ESP32 is being chosen. If I want to run BLE then it will be enabled with a push button. 
Current draw on previous projects has been around 30mA which is ok given the Fridge when on draws 7A. When not connected
the onboard LED on the ESP32 indicates state... and if you want to know if the fridge is on... the compressor makes a sound. 

Control is maintained with 3 states, 0, 1, 2. Charging, Normal, Low Battery. On and off temperatures and compressor rpm
can be set for each state as cant a minimum charging and battery voltage. If the device enters a Low Battery state it can 
transition into the charging state when the minimum charging voltage is exceeded.

Voltages are measured with the ESP32 ADC via a voltage divider that keeps the ADC voltage < 2.5v and so in its linear range. (> 2.5 ESP32s ADC becomes quite non linear). The PWM output uses the wonderfully simple ESP32 led control API. For atMega and atTiny versions see the secoppower.cpp files for those chips. (That code has never been run so expect bugs in it).

The compressor RPM can be varied depending on the difference between measured temperature and target temperature. Temperature is measured over 1wire sensors which are so slow, but fast enough for this use. 1Wire measurements are using asynchronous requests.

# Compressor control.

Fridge circuits work by compressing the refrigerant, cooling it into a liquid form in a condensor, storing the refrigerant liquid in a reservoir. The liquid escapes through a metering device which lowers the pressure of the liquid. On entering the evaporator the liquid boils removing heat from the fridge. The gas returns to the compressor which re-compresses the gas back into a liquid.  Marine fridges tend to use a long capillary tube as the metering device, which is static. The reservwoir is generally quite small. Because the metering is fixed, there is little point in running the compressor at full speed exit temperature of the evaporator is low enough to generate frost, ie < 0C as that will only cool the atomsphere. So a temperature sensor at the exit of the evaporator could be used to control the speed of the compressor to ensure that only enough energy to is put into the compressor to cool the evaporator and not the pipes outside the fridge.  

## Temperature sensor locations.

* Input side of the evaporator, probably central - this is the default location on standard installation.
* Output side if the evaporator - outside the fridge but as far a away from the compressor as possible.
* Interior of fridge.

## Control mechanism.

Control of RPM should be based on the relative temperatures of the evaporator at both ends. The on and off temperature of the fridge should depend on the battery state, with the fridge being off if the voltage falls too low.

# Building

use pio command line and upload.

# Configuring

The serial port runs at 115200 8N1, press h for a menu with options. The PWM duty cycle to rpm mapping will need to be calabrated.


# PCB

Very simple schematic, see files in pcb/

# TODO

- [ ] build pcb
- [ ] hookup and test
- [ ] make use of the variable speed compressor
- [ ] Enable BLE
- [ ] Write Android or Chromium app.




