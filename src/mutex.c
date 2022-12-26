/**
 * Showing the use of Mutex on FreeRTOS.
 * Mutex stands for Mutual Exclusion
 * It is a binary semaphore that includes a priority inheritence mechanism
 *
 * A mutex acts as a Token that is used to guard a resource. Like a lock & key
 * 2 Tasks both need the key to access the resource but only one of them can use it at a time
 * After 1st task finished it returns the key and only then can the 2nd task can also have the key and
 * access the resource.
 *
 * NOTE: This example runs on a single core (Core0)
 */

#include <FreeRTOS.h>
#include <semphr.h>  // Include semaphore header file for Mutex operations
#include <task.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

/// Handle for the semaphore (mutex handle)
static SemaphoreHandle_t mutex;

void mutex_task1(void *pvParameters);
void mutex_task2(void *pvParameters);

/// @brief This should be put in main if you want to test Mutex functionality
/// @return an int exit code
int pretend_main_mutex() {
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

    mutex = xSemaphoreCreateMutex();  // Create the Mutex

    // Create Your Task1
    xTaskCreate(
        mutex_task1,    // Task to be run
        "MUTEX_TASK1",  // Name of the Task for debugging and managing its Task Handle
        256,            // Stack depth to be allocated for use with task's stack (see docs)
        NULL,           // Arguments needed by the Task (NULL because we don't have any)
        1,              // Task Priority - Higher the number the more priority [max is (configMAX_PRIORITIES - 1) provided in FreeRTOSConfig.h]
        NULL            // Task Handle if available for managing the task
    );

    // Create Your Task2
    xTaskCreate(
        mutex_task2,    // Task to be run
        "MUTEX_TASK2",  // Name of the Task for debugging and managing its Task Handle
        256,            // Stack depth to be allocated for use with task's stack (see docs)
        NULL,           // Arguments needed by the Task (NULL because we don't have any)
        1,              // Task Priority - Higher the number the more priority [max is (configMAX_PRIORITIES - 1) provided in FreeRTOSConfig.h]
        NULL            // Task Handle if available for managing the task
    );

    // Should start you scheduled Tasks (such as the LED_Task above)
    vTaskStartScheduler();

    while (true) {
        // Your program should never get here
    };

    return 0;
}

void mutex_task1(void *pvParameters) {
    char ch = '1';
    while (true) {
        // Attempting to take the Mutex
        // If successful it means the semaphore is available for taking
        if (xSemaphoreTake(mutex, 0) == pdTRUE) {
            for (int i = 0; i < 10; i++) {
                putchar(ch);
                vTaskDelay(100);
            }
            puts("");
            xSemaphoreGive(mutex);  // Give back the Mutex
        }
    }
}

void mutex_task2(void *pvParameters) {
    char ch = '1';
    while (true) {
        // Attempting to take the Mutex
        // If successful it means the semaphore is available for taking
        if (xSemaphoreTake(mutex, 0) == pdTRUE) {
            for (int i = 0; i < 10; i++) {
                putchar(ch);
                vTaskDelay(100);
            }
            puts("");
            xSemaphoreGive(mutex);  // Give back the Mutex
        }
    }
}
