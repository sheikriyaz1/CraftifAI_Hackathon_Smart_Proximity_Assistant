cmake_minimum_required(VERSION 3.22)
# Toolchain must be set before project(): -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake
project(ultrasonic_distance_bringup C ASM)
set(CMAKE_C_STANDARD 11)

# CPU flags for the STM32G031K8 used by the NUCLEO-G031K8 board.
set(CPU_FLAGS "-mcpu=cortex-m0plus -mthumb")
separate_arguments(CPU_FLAGS UNIX_COMMAND "${CPU_FLAGS}")

add_compile_options(${CPU_FLAGS} -ffunction-sections -fdata-sections -Wall)
# USE_HAL_DRIVER selects the HAL; STM32G031xx picks the exact device header in CMSIS.
add_compile_definitions(USE_HAL_DRIVER STM32G031xx)

# HAL/core/app sources. The entry point is Core/Src/main.c — do NOT glob src/*.c: a leftover
# src/main.c from an older scaffold would define a second main() and break the link.
# *_template.c files were excluded at copy time (they duplicate symbols).
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/Core/Src/*.c
    ${CMAKE_SOURCE_DIR}/firmware/*.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32G0xx_HAL_Driver/Src/*.c)

set(STARTUP ${CMAKE_SOURCE_DIR}/Core/Startup/startup_stm32g031xx.s)
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32G031K8TX_FLASH.ld)

add_executable(${PROJECT_NAME}.elf ${PROJECT_SOURCES} ${STARTUP})

target_include_directories(${PROJECT_NAME}.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/firmware/configs
    ${CMAKE_SOURCE_DIR}/firmware/app
    ${CMAKE_SOURCE_DIR}/Drivers/STM32G0xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32G0xx/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include)

target_link_options(${PROJECT_NAME}.elf PRIVATE
    ${CPU_FLAGS}
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
    --specs=nano.specs --specs=nosys.specs
    -lc -lm)

# .hex/.bin next to the .elf for flashing.
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin)
