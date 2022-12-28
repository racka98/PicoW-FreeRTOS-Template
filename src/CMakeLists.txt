add_executable(${NAME}  
        blink.c
        blink_queue.c
        scheduling_priorities.c
        mutex.c
        semaphore.c
        ../local-libs/ssd1306/ssd1306.c
        )

# pull in common dependencies
target_link_libraries(${NAME} 
        pico_stdlib              # for core functionality
        pico_cyw43_arch_lwip_threadsafe_background  # we need Wifi
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        hardware_i2c # Hardware I2C
        LWIP_PORT                          # LWIP config files
        FREERTOS_PORT                      # FreeRTOS config files
        )

        
# Enable print functionality on usb & disable on uart
pico_enable_stdio_usb(${NAME}  1) 
pico_enable_stdio_uart(${NAME}  0)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME} )

target_include_directories(${NAME}  PRIVATE ${CMAKE_CURRENT_LIST_DIR} )
