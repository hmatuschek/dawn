set(firmware_SOURCES main.c siphash24.c siphash_2_4_asm.S)
set(firmware_HEADERS siphash24.h siphash_2_4_asm.h)

add_custom_target(firmware SOURCES ${firmware_SOURCES})
