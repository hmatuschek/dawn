# Specify programmer
set(AVR_UPLOADTOOL avrdude)
set(AVR_PROGRAMMER avrispmkII)
set(AVR_UPLOADTOOL_PORT usb)

# Specify CPU
set(AVR_MCU atmega16)

# Specify fuse magic
set(AVR_H_FUSE 0xdf)
set(AVR_L_FUSE 0xc5)

# Specify CPU frequency
set(MCU_SPEED "16000000UL")

# Include avg-gcc toolchain
include(generic-gcc-avr)

# Typical options
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
add_definitions("-Os")

# sources
set(firmware_SOURCES main.c
    siphash24.c pwm.c i2c.c ds1307.c clock.c gpio.c uart.c communication.c)
set(firmware_HEADERS
    siphash24.h pwm.h i2c.h ds1307.h clock.h gpio.h uart.h communication.h)

# Generate LUT for dawn function
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/dawnfunction.h
  COMMAND ${PYTHON_EXECUTABLE}
  ARGS ${CMAKE_SOURCE_DIR}/gen_dawnfunction.py ${CMAKE_BINARY_DIR}/dawnfunction.h
  DEPENDS ${CMAKE_SOURCE_DIR}/gen_dawnfunction.py)

# Make sure firmware uses same secret as application
set_property(SOURCE communication.c APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_BINARY_DIR}/secret.h)
# Make sure dawn function gets generated
set_property(SOURCE clock.c APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_BINARY_DIR}/dawnfunction.h)

# compile & link
add_avr_executable(firmware ${firmware_SOURCES})
