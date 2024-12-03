cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

IF (CMAKE_HOST_WIN32)
set(picotool_DIR ${CMAKE_CURRENT_LIST_DIR}/tools/picotool/lib/cmake/picotool)
ENDIF ()

set(PICO_NO_COPRO_DIS 1)

set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/freertos)
include(freertos/portable/ThirdParty/GCC/RP2350_ARM_NTZ/FreeRTOS_Kernel_import.cmake)

project(debugprobe)

pico_sdk_init()

if (${PICO_SDK_VERSION_MAJOR} LESS 2)
	message(SEND_ERROR "Version 2 of the Pico SDK is required to compile this project. Please update your installation at ${PICO_SDK_PATH}")
endif ()

add_executable(debugprobe
        src/probe_config.c
        src/led.c
        src/main.c
        src/usb_descriptors.c
        src/probe.c
        #src/cdc_uart.c
        src/cdc_uart_v2.c
        src/ringbuf.c
        src/get_serial.c
        src/sw_dp_pio.c
        src/tusb_edpt_handler.c
)

target_sources(debugprobe PRIVATE
        CMSIS-DAP/Firmware/Source/DAP.c
        CMSIS-DAP/Firmware/Source/JTAG_DP.c
        CMSIS-DAP/Firmware/Source/DAP_vendor.c
        CMSIS-DAP/Firmware/Source/SWO.c
        #CMSIS-DAP/Firmware/Source/SW_DP.c
        )

file(GLOB BLACKMAGIC_FILE_LIST 
        blackmagic/src/target/*.c
        blackmagic/src/command.c
        blackmagic/src/crc32.c
        blackmagic/src/exception.c
        blackmagic/src/gdb_main.c
        blackmagic/src/gdb_packet.c
        blackmagic/src/hex_utils.c
        blackmagic/src/maths_utils.c
        blackmagic/src/morse.c
        blackmagic/src/remote.c
        blackmagic/src/rtt.c
        blackmagic/src/timing.c
        src/bmp_port/*.c
        )
file(GLOB EXCLUDE_FILE_LIST
        blackmagic/src/target/jtagtap_generic.c
        blackmagic/src/target/swdptap_generic.c
        )
list(REMOVE_ITEM BLACKMAGIC_FILE_LIST ${EXCLUDE_FILE_LIST})

target_sources(debugprobe PRIVATE
        ${BLACKMAGIC_FILE_LIST}
        )

target_include_directories(debugprobe PRIVATE
        CMSIS-DAP/Firmware/Include/
        CMSIS_6/CMSIS/Core/Include/
        include/
        blackmagic/src/
        blackmagic/src/include/
        blackmagic/src/target/
        src/bmp_port/
        )

# For blackmagic
target_compile_definitions(debugprobe PRIVATE 
        NO_LIBOPENCM3
        ENABLE_CORTEXAR=1
        ENABLE_XILINX=1
        ENABLE_RISCV=1
        ENABLE_RISCV_ACCEL=1
        ENABLE_RTT
        )

target_compile_options(debugprobe PRIVATE -Wall)

pico_generate_pio_header(debugprobe ${CMAKE_CURRENT_LIST_DIR}/src/probe.pio)
pico_generate_pio_header(debugprobe ${CMAKE_CURRENT_LIST_DIR}/src/probe_oen.pio)

target_include_directories(debugprobe PRIVATE src)

target_compile_definitions (debugprobe PRIVATE
	PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1
)

target_link_options(debugprobe PRIVATE -Wl,--print-memory-usage)

option (DEBUG_ON_PICO "Compile firmware for the Pico instead of Debug Probe" OFF)
if (DEBUG_ON_PICO)
    target_compile_definitions (debugprobe PRIVATE 
	DEBUG_ON_PICO=1
    )
    set_target_properties(debugprobe PROPERTIES 
        OUTPUT_NAME "debugprobe_on_pico"
    )
endif ()


target_link_libraries(debugprobe PRIVATE
        pico_multicore
        pico_stdlib
        pico_unique_id
        tinyusb_device
        tinyusb_board
        hardware_pio
        hardware_dma
        hardware_adc
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap1
)

# pico_set_binary_type(debugprobe copy_to_ram)

pico_add_extra_outputs(debugprobe)
