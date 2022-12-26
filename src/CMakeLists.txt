add_executable(src 
        blink.c
        blink_queue.c
        )

# pull in common dependencies
target_link_libraries(src
        pico_stdlib              # for core functionality
        pico_cyw43_arch_lwip_threadsafe_background  # we need Wifi
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        )

        
# Enable print functionality on usb & disable on uart
pico_enable_stdio_usb(src 1) 
pico_enable_stdio_uart(src 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(src)

target_include_directories(src PRIVATE ${CMAKE_CURRENT_LIST_DIR} )
