# Bluetooth Controlled light alarm-clock


## Communication protocol

Basic structure command structure
 +--------+-- ... --+-- ... --+
 |  CMD   | PAYLOAD |   MAC   |
 +--------+---... --+-- ... --+

Each command message consists of a 8-bit command field, an optional Payload (variable size)
and the signature (64 bit). The only exception forms the SALT command, returning the current
salt for the signature of the next command.

Basic response structure
 +--------+-- ... --+-- ... --+
 |  RESP  | PAYLOAD |   MAC   |
 +--------+-- ... --+-- ... --+

Just for fun, also the responses are signed usig the IV obained from the command
signature.


### GET_SALT command (CMD = 0x00)
Returns the current salt (64bit) used for the computation of the signature of the next command.
The salt changes each time, a command is received or a response is send by the device.
The SALT command message consists of a single byte (value 0x00) without any signature.

The SALT response has not the form shown above, it simply is the current SALT.


### MAC algorithm

The signature (128bit) is simply the SipHash-2-4 of the message body

  MAC <- siphash-2-4_cbc(LAST_MAC, MESSAGE, SECRET)

A CBC-MAC was chosen to avoid replay attacks, in the case of an unencrypted chanel.


### GET_VALUE command (0x01)

Returns the current value of the lamp. This command has no payload.

The response contains the current value as a uint8_t as the payload.


### SET_VALUE command (0x02)

Sets the current value of the lamp.

The command payload contains the value as a uint8_t.

The response has no payload.


### GET_TIME command (0x03)

Returns the current date and time of the RTC.

The command has no payload.

The response payload has the following structure

YEAR:MONTH:DAY:HOUR:MINTUE:SECOND

Where YEAR is transmitted as a uint16_t and MONTH, HOUR, MINUTE and SECOND as uint8_t.


### SET_TIME command (0x04)

Set the current date and time of the RTC.

The command payload has the following structure

YEAR:MONTH:DAY:HOUR:MINTUE:SECOND

Where YEAR is transmitted as a uint16_t and MONTH, HOUR, MINUTE and SECOND as uint8_t.

The response has no payload.


### NUM_ALARM command (0x05)

Returns the number of possible alarm settings.


### GET_ALARM command (0x06)

Returns the settings for the i-th alarm, where i in [0,NUM_ALARM)

The payload contains the index of the alarm as a uint8_t.

The response Payload has the following format

RESERVED:DAY_OF_WEEK:ENABLED:HOUR:MINUTE

The first 4 bits (MSBs) are reserved, followed by 3 bits encoding the day of the week:

0b000 - Every day
0b001 - Sunday
0b010 - Monday
0b011 - Tuesday
0b100 - Wednesday
0b101 - Thursday
0b110 - Friday
0b111 - Saturday

ENABLED is a single bit indicating if the alarm setting is enabled. HOUR and MINUTE are encoded as
uint8_t.


### SET_ALARM command (0x07)

Sets the i-th alarm settings, where i in [0,N_ALARM).

The command payload has the following format:

I:RESERVED:DAYOFWEEK:ENABLED:HOUR:MINUTE

Where I is encodes the alarm index as uint8_t. The 4 RESERVED bits (MSBs) are ignored. DAYOFWEEK
are 3 bits encoding the day of the week:

0b000 - Every day
0b001 - Sunday
0b010 - Monday
0b011 - Tuesday
0b100 - Wednesday
0b101 - Thursday
0b110 - Friday
0b111 - Saturday

ENABLED is a single bit indicating if the alarm setting is enabled. HOUR and MINUTE are encoded as
uint8_t.


### GET_MAXDIM (0x08)

Returns the maximum dimming value.

The command has no payload.

The response payload contains of a single uint8_t specifying the maximum dim value.


### SET_MAXDIM (0x09)

Set the maximum dimming value.

The command payload contains a single uint8_t specifying the maximum dim value.

The response has no payload.


### GET_DIM_DUR (0x0A)

Returns the dim duration in minutes.

The command has no payload.

The response contains a single uint8_t specifying the dimming duration in minutes.


### SET_DIM_DUR (0x0B)

Set the dimming duration in minutes.

The command payload contains a single uint8_t specifying the dimming duration in minutes.

The response has no payload.
