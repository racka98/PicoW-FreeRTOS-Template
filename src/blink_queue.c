/**
 * Showing the Tasks Queue functionality on FreeRTOS.
 *
 * NOTE: Two Tasks are scheduled but both run on a single core (Core0)
 */

#include <FreeRTOS.h>
#include <queue.h>  // Queue functionality from here
#include <task.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

// Gloabal Queue handle
static QueueHandle_t xQueue = NULL;

void led_task_queue(void *pvParameters);  // Task 1
void usb_task_queue(void *pvParameters);  // Task 2

/// @brief This should be put in main if you want to test the Queue functionality
/// @return an int exit code
int pretend_main_queue() {
    stdio_init_all();  // Initialize

    // Create your Queue
    xQueue = xQueueCreate(
        1,            // Length of the Queue (we set one because we only want one item in the Queue)
        sizeof(uint)  // Size of the item(s) stored
    );

    // Create Your LED Task
    xTaskCreate(
        led_task_queue,  // Task to be run
        "LED_TASK",      // Name of the Task for debugging and managing its Task Handle
        256,             // Stack depth to be allocated for use with task's stack (see docs)
        NULL,            // Arguments needed by the Task (NULL because we don't have any)
        1,               // Task Priority - Higher the number the more priority [max is (configMAX_PRIORITIES - 1) provided in FreeRTOSConfig.h]
        NULL             // Task Handle if available for managing the task
    );

    // Create Your USB Task
    xTaskCreate(
        usb_task_queue,  // Task to be run
        "USB_TASK",      // Name of the Task for debugging and managing its Task Handle
        256,             // Stack depth to be allocated for use with task's stack (see docs)
        NULL,            // Arguments needed by the Task (NULL because we don't have any)
        1,               // Task Priority - Higher the number the more priority [max is (configMAX_PRIORITIES - 1) provided in FreeRTOSConfig.h]
        NULL             // Task Handle if available for managing the task
    );

    // Should start you scheduled Tasks (such as the LED_Task above)
    vTaskStartScheduler();

    while (true) {
        // Your program should never get here
    };
}

void led_task_queue(void *pvParameters) {
    bool isConnected = true;
    if (cyw43_arch_init()) {
        printf("WiFi init failed");
        isConnected = false;
    }

    uint uIValueToSend = 0;  // LED Value to be sent to the USB_TASK

    while (isConnected) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        uIValueToSend = 1;
        // Sending ON data to the queue
        xQueueSend(
            xQueue,          // The queue you created
            &uIValueToSend,  // Pointer for value to send to the queue
            0U               // Delay for sending to queue (0 for now since our queue should be empty)
        );
        vTaskDelay(100);  // Delay by TICKS defined by FreeRTOS priorities

        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        uIValueToSend = 0;
        // Sending OFF data to the queue
        xQueueSend(
            xQueue,          // The queue you created
            &uIValueToSend,  // Pointer for value to send to the queue
            0U               // Delay for sending to queue (0 for now since our queue should be empty immediately)
        );
        vTaskDelay(100);
    }
}

void usb_task_queue(void *pvParameters) {
    uint uIReceiveValue;  // Value to retrieve from the Queue

    while (true) {
        // Receiving LED data from the queue
        xQueueReceive(
            xQueue,           // The queue you created
            &uIReceiveValue,  // Pointer for value to store the received data from the queue
            portMAX_DELAY     // Delay for sending to queue (we set max delay to wait for the value)
        );

        // It may be NULL if no value was written to it. So we check for both scenarios
        if (uIReceiveValue == 1) {
            printf("LED Turned ON!\n");
        } else if (uIReceiveValue == 0) {
            printf("LED Turned OFF!\n");
        }
    }
}
