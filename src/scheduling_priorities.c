/**
 * Showing the Task Scheduling & Handling Priorities on FreeRTOS.
 * NOTE: This example runs on a single core (Core0)
 */

#include <FreeRTOS.h>
#include <task.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

void task1(void *pvParameters);  // Task 1
void task2(void *pvParameters);  // Task 2

/// @brief This should be put in main if you want to test the scheduling & priority functionality
/// @return an int exit code
int pretend_main_scheduling_and_priorities() {
    stdio_init_all();  // Initialize

    bool isConnected = true;
    if (cyw43_arch_init()) {
        printf("WiFi init failed");
        isConnected = false;
    }
    // Turn on the LED on start
    if (isConnected) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        printf("LED Turned ON!");
    }

    // Create Your Task1
    xTaskCreate(
        task1,    // Task to be run
        "TASK1",  // Name of the Task for debugging and managing its Task Handle
        256,      // Stack depth to be allocated for use with task's stack (see docs)
        NULL,     // Arguments needed by the Task (NULL because we don't have any)
        1,        // Task Priority - Higher the number the more priority [max is (configMAX_PRIORITIES - 1) provided in FreeRTOSConfig.h]
        NULL      // Task Handle if available for managing the task
    );

    // Create Your Task2
    xTaskCreate(
        task2,    // Task to be run
        "TASK2",  // Name of the Task for debugging and managing its Task Handle
        256,      // Stack depth to be allocated for use with task's stack (see docs)
        NULL,     // Arguments needed by the Task (NULL because we don't have any)
        2,        // Task Priority - Higher the number the more priority [max is (configMAX_PRIORITIES - 1) provided in FreeRTOSConfig.h]
        NULL      // Task Handle if available for managing the task
    );

    // Should start you scheduled Tasks (such as the LED_Task above)
    vTaskStartScheduler();

    while (true) {
        // Your program should never get here
    };

    return 0;
}

// Priority should be 1
void task1(void *pvParameters) {
    while (true) {
        printf("Task 1 is running...\n");
        // Uncomment to see the differences

        // Task 1 will not run since Task 2 has higher priority
        // Task 1 will be in blocked state
        for (int i = 0; i < 20000000; i++) {
            // Just simulating some long operation (we don't use delays)
        }

        // Task 1 & 2 will both run, alternating btn eachother
        // If Task 1 & 2 both have the same priority they will alternate using "Time Slicing"
        // Change priority in the xTaskCreate and see the results
        // vTaskDelay(100);
    }
}

// Priority should be 2
void task2(void *pvParameters) {
    while (true) {
        printf("Task 2 is running...\n");
        // Uncomment to see the differences

        // Task 2 will be dominant since it has higher priority
        for (int i = 0; i < 20000000; i++) {
            // Just simulating some long operation (we don't use delays)
        }

        // Task 1 & 2 will both run, alternating btn eachother
        // If Task 1 & 2 both have the same priority they will alternate using "Time Slicing"
        // Change priority in the xTaskCreate and see the results
        // vTaskDelay(100);
    }
}
