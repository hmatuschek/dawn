set(AVR_UPLOADTOOL avrdude)
set(AVR_PROGRAMMER avrispmkII)
set(AVR_UPLOADTOOL_PORT usb)

set(AVR_MCU atmega16)
set(AVR_H_FUSE 0xd9)
set(AVR_L_FUSE 0xc3)

include(generic-gcc-avr)

set(MCU_SPEED "16000000UL")

add_definitions("-DF_CPU=${MCU_SPEED}")
add_definitions("-fpack-struct")
add_definitions("-fshort-enums")
add_definitions("-Wall")
add_definitions("-Werror")
add_definitions("-pedantic")
add_definitions("-pedantic-errors")
add_definitions("-funsigned-char")
add_definitions("-funsigned-bitfields")
add_definitions("-ffunction-sections")
add_definitions("-c")
add_definitions("-std=gnu99")



set(firmware_SOURCES main.c
    siphash24.c pwm.c i2c.c ds1307.c clock.c gpio.c uart.c communication.c)
set(firmware_HEADERS
    siphash24.h pwm.h i2c.h ds1307.h clock.h gpio.h uart.h communication.h)

add_avr_executable(firmware ${firmware_SOURCES})
