# Bluetooth controlled dawn simulator

This is a simple bluetooth-controlled bedside light which can act as a dawn simulator.

## Hardware

The hardware (schematics can be found in the hardware/ directory) consists of a high-power LED,
a current regulator (BUZ11 + 2 BC547), an ATMega168-22P MCU, a real-time clock (DS1307) with
back-up battery and a serial <-> bluetooth interface module (JY-MCU).

The circuit can be adopted easily to different LEDs by changing the value of R25 and/or R30 such
that R25||R30 = 0.6/Imax, where Imax is the maximum current through the LED. Please be aware that
the MOSFET Q4 as well as the LED may need some cooling.

The brightness of the LED will be controlled by the high-speed PWM (pin 15) of the ATMega
through the low-pass (R27,C1).

The firmware for the ATMega168 can be found in the firmware/ directory.


## Software

The software can be found in the src/ directory an provides a simple GUI using QT5 to configure and
controll the dawn simulator.


## Communication protocol

Basic structure command structure
 +--------+-- ... --+-- ... --+
 |  CMD   | PAYLOAD |   MAC   |
 +--------+---... --+-- ... --+

Each command message consists of a 8-bit command field, an optional Payload (variable size)
and a message authentication code (64 bit).

The serial <-> bluetooth module has only a 4-digit pin for the access control. Hence some
additional measures are needed for the access control. Every message send to the device is
therefore authenticated using a simple MAC. The MAC is computed over the CMD and PAYLOAD fields
of the message using the SIPHASH(2,4) function with a shared secret. This does not provide
a very high level of security but is sufficient for a bedside light.


### MAC algorithm

The signature (64bit) is simply the SipHash-2-4 of the message body

  MAC = siphash24(MESSAGE, SECRET)


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

The first bit (MSB) is reserved (0), followed by 7 bits encoding the day of the week:

0b0000001 - Sunday
0b0000010 - Monday
0b0000100 - Tuesday
0b0001000 - Wednesday
0b0010000 - Thursday
0b0100000 - Friday
0b1000000 - Saturday

HOUR and MINUTE are encoded as uint8_t.


### SET_ALARM command (0x06)

Sets the i-th alarm settings, where i in [0,6].

The command payload has the following format:

IDX:DOW_FLAGS:HOUR:MINUTE

Where IDX is encodes the alarm index as uint8_t. DOW_FALGS 7 least significant bits encoding the day of the week:

0b0000001 - Sunday
0b0000010 - Monday
0b0000100 - Tuesday
0b0001000 - Wednesday
0b0010000 - Thursday
0b0100000 - Friday
0b1000000 - Saturday

HOUR and MINUTE are encoded as uint8_t.
