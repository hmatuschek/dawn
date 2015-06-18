# Bluetooth controlled dawn simulator

This is a simple Bluetooth controlled bedside light which can act as a dawn simulator.

**Disclaimer**: The hardware schematics provided here deal with relatively high currents 
(depending on the LED used, >1A) which may even cause fire. If you rebuild the hardware,
you do that on our own risk! I am not responsible for any damage caused by the circuits 
or the software provided here. In general, do not rebuild the hardware if you do not 
understand the circuit completely!


## Hardware

The hardware ([schematics](https://github.com/hmatuschek/dawn/blob/master/doc/lampe_brd.pdf) 
can be found in the hardware/ directory) consists of a high-power LED, a current regulator
(BUZ11 + 2x BC547),  an ATMega168 MCU, a real-time clock (DS1307) with back-up battery
and a RS232 Bluetooth interface module (JY-MCU).

The [circuit](https://github.com/hmatuschek/dawn/blob/master/doc/lampe_brd.pdf) can be 
adopted easily to different LEDs by changing the value of R25 and/or R30 such that 
R25||R30 = 0.6/Imax, where Imax is the maximum current through the LED. Please be aware 
that the MOSFET Q4 as well as the LED may need some cooling.

The brightness of the LED will be controlled by the PWM output (pin 15) of the ATMega.

The firmware for the ATMega168 can be found in the firmware/ directory.


## Software

The software can be found in the src/ directory and provides a simple GUI using QT5 to configure and
control the dawn simulator. 

The dawn simulator is interfaced via Bluetooth, hence you must first connect to the device using the 
OS Bluetooth configuration. This will create a new serial port (RS232) which needs to be selected 
within the GUI application. 


## Communication protocol

The communication protocol is a trivial binary one. Every command/request is transmitted in terms of 
messages. The basic message structure is

    +--------+-- ... --+-- ... --+
    |  CMD   | PAYLOAD |   MAC   |
    +--------+---... --+-- ... --+

Each command message consists of a 8-bit command field, an optional Payload (variable size)
and a message authentication code (64 bit).

The RS232 Bluetooth module has only a 4-digit PIN for the access control. Hence some
additional measures are needed. Every message send to the device is therefore authenticated
using a simple MAC. The MAC is computed over the CMD and PAYLOAD fields of the message 
using the SIPHASH(2,4) function with a shared secret. This does not provide a very high 
level of security but may be considered sufficient for a bedside light.

Please note that the device will not send any response on an error. I.e. if an invalid command is received 
or if the MAC is invalid.

### (H)MAC algorithm

The signature (64bit) is simply the SipHash-2-4 of the message body with a shared 128bit secret.

  MAC = siphash24(MESSAGE, SECRET)

Please note that there is no IV that changes with every command. Hence the protocol is in principle 
vulnerable to replay attacks. Although the Bluetooth authentication is secured using a trivial 
4-digit PIN, the actual communication is encrypted with a relatively strong algorithm and key. 
Unless an attacker is able to capture the pairing, the communication should be relatively robust 
against trivial replay attacks. 

### GET_VALUE command (0x01)

Returns the current luminescence value of the lamp. This command has no payload.

The response contains the current value as a uint16_t as the payload.

### SET_VALUE command (0x02)

Sets the current luminescence value of the lamp.

The command payload contains the value as a uint16_t.

On success, a single byte (0x00) is returned.

### GET_TIME command (0x03)

Returns the current date and time of the RTC.

The command has no payload.

The response payload has the following structure

YEAR:MONTH:DAY:DAYOFWEEK:HOUR:MINTUE:SECOND

Where YEAR is transmitted as a uint16_t and MONTH, DAY, DAYOFWEEK, HOUR, MINUTE and SECOND as
uint8_t.

### SET_TIME command (0x04)

Set the current date and time of the RTC.

The command payload has the following structure

YEAR:MONTH:DAY:DAYOFWEEK:HOUR:MINTUE:SECOND

Where YEAR is transmitted as a uint16_t and MONTH, DAY, DAYOFWEEK, HOUR, MINUTE and SECOND as
uint8_t.

On success, a single byte (0x00) is returned.

### GET_ALARM command (0x05)

Returns the settings for the i-th alarm, where i in [0,6]

The payload contains the index of the alarm as a uint8_t.

The response Payload has the following format

DOW_FLAGS:HOUR:MINUTE

The first bit (MSB) is reserved (0), followed by 7 bits encoding the day of the week at which 
the alarm is enabled. 

0b0000001 - Sunday
0b0000010 - Monday
0b0000100 - Tuesday
0b0001000 - Wednesday
0b0010000 - Thursday
0b0100000 - Friday
0b1000000 - Saturday

Hence a value of 0x7f will enable the alarm on every day and a value of 0x00 will effectively disable
the alarm.

HOUR and MINUTE are encoded as uint8_t.

### SET_ALARM command (0x06)

Sets the i-th alarm settings, where i in [0,6].

The command payload has the following format:

IDX:DOW_FLAGS:HOUR:MINUTE

Where IDX is encodes the alarm index as uint8_t. The 7 least significant bits of DOW_FLAGS encode 
the day of the week at which the alarm is enabled.

0b0000001 - Sunday
0b0000010 - Monday
0b0000100 - Tuesday
0b0001000 - Wednesday
0b0010000 - Thursday
0b0100000 - Friday
0b1000000 - Saturday

HOUR and MINUTE are encoded as uint8_t.


## License

dawn - A dawn-simulating bedside light.

Copyright (C) 2015  Hannes Matuschek

This program and harware is "free software"; you can redistribute it 
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program and hardware is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.