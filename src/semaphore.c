/**
 * Showing the use of Binary Semaphore on FreeRTOS.
 * Its like a Mutex but without Priority inheritence
 * Its Queue can hold only one (1) item with value of either 0 or 1
 *
 * Binary Semaphores are best for implementing task synchronization.
 * While Mutex are best for implementing Mutual Exclusion or resource protection.
 *
 * NOTE: This example runs on a single core (Core0)
 */

#include <FreeRTOS.h>
#include <semphr.h>  // Include semaphore header file for Semaphore operations
#include <task.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#define BUTTON_GPIO 20  // Connect a Button to GPIO20 of the board

/// Handle for the count semaphore
static SemaphoreHandle_t count;

void sem_led_task(void *pvParameters);
void button_task(void *pvParameters);

/// @brief This should be put in main if you want to test Binary Semaphore functionality
/// @return an int exit code
int pretend_main_semaphore() {
    stdio_init_all();  // Initialize

    // Create the counting Semaphore
    count = xSemaphoreCreateCounting(
        5,  // Max count
        0   // Starting count
    );

    // Create Your LED TASK
    xTaskCreate(
        sem_led_task,    // Task to be run
        "SEM_LED_TASK",  // Name of the Task for debugging and managing its Task Handle
        256,             // Stack depth to be allocated for use with task's stack (see docs)
        NULL,            // Arguments needed by the Task (NULL because we don't have any)
        1,               // Task Priority - Higher the number the more priority [max is (configMAX_PRIORITIES - 1) provided in FreeRTOSConfig.h]
        NULL             // Task Handle if available for managing the task
    );

    // Create Your Task2
    xTaskCreate(
        button_task,    // Task to be run
        "BUTTON_TASK",  // Name of the Task for debugging and managing its Task Handle
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

void sem_led_task(void *pvParameters) {
    bool is_connected = true;
    if (cyw43_arch_init()) {
        printf("WiFi init failed");
        is_connected = false;
    }

    while (is_connected) {
        // If can take semaphore it means the button has been pressed
        // Since the button gives the semaphore
        if (xSemaphoreTake(count, (TickType_t)10) == pdTRUE) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            vTaskDelay(100);
        } else {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            vTaskDelay(1);
        }
    }
}

void button_task(void *pvParameters) {
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);

    while (true) {
        if (gpio_get(BUTTON_GPIO) != 0) {
            // Give to semaphore to prevent mulitple button presses
            xSemaphoreGive(count);
            vTaskDelay(20);
        } else {
            vTaskDelay(1);
        }
    }
}
