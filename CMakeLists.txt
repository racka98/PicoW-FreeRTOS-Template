cmake_minimum_required(VERSION 3.12)

# Change your executable name to something creative!
set(NAME PICO_FREERTOS) # <-- Name your project/executable here!

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_freertos C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w) # Set the board used

# Set config files paths to be used
include(lwip_import.cmake) # lwip config files
SET(FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/configs/FreeRTOS-Kernel" CACHE STRING "Local Config")
include_directories("${FREERTOS_CONFIG_FILE_DIRECTORY}") # FreeRTOS config files

# Initialize the SDK
pico_sdk_init()

# Initialize all other external libs & dependencies
include(FreeRTOS_Kernel_import.cmake)

add_subdirectory(src)
