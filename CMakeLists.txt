cmake_minimum_required(VERSION 3.5)

set(FREERTOS_KERNEL_PATH "/home/karthik/FreeRTOS-Kernel")
set(PICO_SDK_PATH "/home/karthik/pico-sdk")
set(PICOTOOL_FETCH_FROM_GIT_PATH "/home/karthik/picotool")
set(PICO_PLATFORM rp2040)
set(PICO_BOARD pico)

# Pull in SDK (must be before project)
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Pull in FreeRTOS
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(rpi2040 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(rpi2040
    core_0/core_0.c
    core_1/core_1.c
    os_sched/os_sched.c
)

# Add include directories
target_include_directories(rpi2040
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/core_0
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/core_1
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/os_sched
    PUBLIC ${FREERTOS_KERNEL_PATH}/include
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(rpi2040 
FreeRTOS-Kernel 
FreeRTOS-Kernel-Heap4 
pico_stdlib
hardware_adc
hardware_pwm
)

# enable usb output, disable uart output
pico_enable_stdio_usb(rpi2040 1)
pico_enable_stdio_uart(rpi2040 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(rpi2040)
