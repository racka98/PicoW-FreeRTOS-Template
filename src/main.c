
#include <FreeRTOS.h>
#include <task.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

void led_task();

int main() {
    stdio_init_all();  // Initialize

    // Create Your Task
    xTaskCreate(
        led_task,    // Task to be run
        "LED_Task",  // Name of the Task for debugging and managing its Task Handle
        256,         // Stack depth to be allocated for use with task's stack (see docs)
        NULL,        // Arguments needed by the Task (NULL because we don't have any)
        1,           // Task Priority - Higher the number the more priority [max is (configMAX_PRIORITIES - 1) provided in FreeRTOSConfig.h]
        NULL         // Task Handle if available for managing the task
    );

    // Should start you scheduled Tasks (such as the LED_Task above)
    vTaskStartScheduler();

    while (true) {
        // Your program should never get here
    };
}

void led_task() {
    if (cyw43_arch_init()) {
        printf("WiFi init failed");
        return;
    }

    while (true) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        vTaskDelay(100);  // Delay by TICKS defined by FreeRTOS priorities
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        vTaskDelay(100);
    }
}
