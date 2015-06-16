# Bluetooth Controlled light alarm-clock


## Communication protocol

Basic structure command structure
 +--------+-- ... --+-- ... --+
 |  CMD   | PAYLOAD |   MAC   |
 +--------+---... --+-- ... --+

Each command message consists of a 8-bit command field, an optional Payload (variable size)
and the signature (64 bit). 


### GET_SALT command (CMD = 0x00)
Returns the current salt (64bit) used for the computation of the signature of the next command.
The salt changes each time, a command is received or a response is send by the device.
The SALT command message consists of a single byte (value 0x00) without any signature.

The SALT response has not the form shown above, it simply is the current SALT.


### MAC algorithm

The signature (64bit) is simply the SipHash-2-4 of the message body

  MAC = siphash24(MESSAGE, SECRET)

### GET_VALUE command (0x01)

Returns the current value of the lamp. This command has no payload.

The response contains the current value as a uint8_t as the payload.


### SET_VALUE command (0x02)

Sets the current value of the lamp.

The command payload contains the value as a uint8_t.

On success, a single byte (0x00) is returned.


### GET_TIME command (0x03)

Returns the current date and time of the RTC.

The command has no payload.

The response payload has the following structure

YEAR:MONTH:DAY:DAYOFWEEK:HOUR:MINTUE:SECOND

Where YEAR is transmitted as a uint16_t and MONTH, DAY, DAYOFWEEK, HOUR, MINUTE and SECOND as uint8_t.


### SET_TIME command (0x04)

Set the current date and time of the RTC.

The command payload has the following structure

YEAR:MONTH:DAY:DAYOFWEEK:HOUR:MINTUE:SECOND

Where YEAR is transmitted as a uint16_t and MONTH, DAY, DAYOFWEEK, HOUR, MINUTE and SECOND as uint8_t.

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
