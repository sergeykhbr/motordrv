cmake_minimum_required(VERSION 3.20)
project(motordrv LANGUAGES C CXX ASM)

set(COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../common")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_executable(motordrv)

target_sources(motordrv PRIVATE
    ${COMMON_DIR}/system/stm32f4xx_isr_vector.c
    ${COMMON_DIR}/system/system_handlers.c
    ${COMMON_DIR}/system/system_init.c
    ${COMMON_DIR}/system/gcc_startup.c
    ${COMMON_DIR}/system/gcc_newlib.c
    ${COMMON_DIR}/system/vprintfmt.c
    src/system/app_handlers.cpp
    src/system/uart_early_init.cpp
    ${COMMON_DIR}/fw/fwmem.c
    ${COMMON_DIR}/fw/fwapi.cpp
    ${COMMON_DIR}/fw/fwattribute.cpp
    ${COMMON_DIR}/fw/fwobject.cpp
    ${COMMON_DIR}/fw/fwfifo.cpp
    src/drivers/fwkernelgen.cpp
    src/drivers/gpio_drv.c
    src/drivers/user_btn.cpp
    src/drivers/relais.cpp
    src/drivers/ledstrip.cpp
    src/drivers/rtc_drv.cpp
    src/drivers/usrsettings.cpp
    src/drivers/display_ili9341.cpp
    src/drivers/display_spi.cpp
    src/system/app_fwkernel.cpp
    src/ManagementClass.cpp
    src/fwmain.cpp
)

target_include_directories(motordrv PRIVATE
    "${COMMON_DIR}/include"
    "${COMMON_DIR}/include/devs"
    "${COMMON_DIR}/system"
    "${COMMON_DIR}/fw"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/system"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/drivers"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

set_target_properties(motordrv PROPERTIES
    OUTPUT_NAME "motordrv"
    SUFFIX ".elf"
)

set(ARM_CPU_FLAGS
    -mcpu=cortex-m4
    -mfloat-abi=hard
)

set(COMMON_COMPILE_FLAGS
    -g
    -Os
    -static
    -fno-common
    -DxSHOW_DBC
)

target_compile_options(motordrv PRIVATE
    ${ARM_CPU_FLAGS}
    ${COMMON_COMPILE_FLAGS}
    $<$<COMPILE_LANGUAGE:C>:-std=gnu99>
    $<$<COMPILE_LANGUAGE:ASM>:-D__ASSEMBLY__=1>
)

target_link_options(motordrv PRIVATE
    ${ARM_CPU_FLAGS}
    -nostartfiles
    "-T${COMMON_DIR}/system/stm32f4xx.ld"
)

target_link_libraries(motordrv PRIVATE
    gcc
    stdc++
    c
    m
)

# Post compilation:
add_custom_command(TARGET motordrv POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        "-DELF_FILE=$<TARGET_FILE:motordrv>"
        "-DOUTPUT_DIR=${CMAKE_BINARY_DIR}"
        "-DOBJDUMP=${CMAKE_OBJDUMP}"
        "-DOBJCOPY=${CMAKE_OBJCOPY}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/GenerateArtifacts.cmake"
    COMMENT "Generating motordrv.dump, motordrv.lst and motordrv.bin"
    VERBATIM
)

message(STATUS "--------------------------------------")
message(STATUS "Motor Driver configuration")
message(STATUS "--------------------------------------")
message(STATUS "Build type:    ${CMAKE_BUILD_TYPE}")
message(STATUS "Linker script: ${LINKER_SCRIPT}")
message(STATUS "Output:        ${CMAKE_BINARY_DIR}")
message(STATUS "--------------------------------------")
