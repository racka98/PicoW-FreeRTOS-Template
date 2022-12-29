/**
 * This example uses a SSD1306 128x64 Display
 * It is connected to GPI0 4 (for SDA) and GPIO 5 (for SCL)
 * You can change the defined pins in DISPLAY_SDA & DISPLAY_SCL definitions
 *
 * This should do the following in a loop;
 * - Send value to turn ON LED
 * - Calculate temp & voltage
 * - Display on SSD1306
 * - Flash the display
 * - Send value to turn OFF LED
 */

#include "temp_display_queue.h"

#include <FreeRTOS.h>
#include <hardware/adc.h>
#include <hardware/i2c.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>
#include <queue.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <task.h>

#include "ssd1306.h"
#include "string_operations.h"

#define DISPLAY_SDA 4
#define DISPLAY_SCL 5

static const float CONVERSION_FACTOR = 3.3f / (1 << 12);

// Gloabal Queue handle
static QueueHandle_t led_queue = NULL;

static void on_board_temp_task(void *pvParameters);  // After calc flash LED
static void led_flash_task(void *pvParameters);

static void setup_display_gpio();
static void write_temp_to_display(ssd1306_t *disp, float *temp, float *voltage);
static void send_queue_value(uint *value);
static void recieve_queue_value(uint *value);

void create_temp_display_queue_task() {
    // Create your Queue
    led_queue = xQueueCreate(
        1,            // Length of the Queue (we set one because we only want one item in the Queue)
        sizeof(uint)  // Size of the item(s) stored
    );

    // Create Your Task
    xTaskCreate(
        on_board_temp_task,  // Task to be run
        "DISPLAY_TEMP",      // Name of the Task for debugging and managing its Task Handle
        1024,                // Stack depth to be allocated for use with task's stack (see docs)
        NULL,                // Arguments needed by the Task (NULL because we don't have any)
        1,                   // Task Priority
        NULL                 // Task Handle if available for managing the task
    );

    // Create Your Task
    xTaskCreate(
        led_flash_task,      // Task to be run
        "LED_TRIGGER_TEMP",  // Name of the Task for debugging and managing its Task Handle
        256,                 // Stack depth to be allocated for use with task's stack (see docs)
        NULL,                // Arguments needed by the Task (NULL because we don't have any)
        1,                   // Task Priority
        NULL                 // Task Handle if available for managing the task
    );

    // Should start you scheduled Tasks (such as the LED_Task above)
    vTaskStartScheduler();

    while (true) {
        // Your program should never get here
    };
}

static void on_board_temp_task(void *pvParameters) {
    adc_init();  // initialize ADC
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);  // Take the fifth channel of the ADC

    // init display
    printf("Configuring GPIO PINS\n");
    setup_display_gpio();
    ssd1306_t display;
    display.external_vcc = false;
    ssd1306_init(&display, 128, 64, 0x3C, i2c0);

    uint ledSendValue = 0;  // LED Value to be sent

    while (true) {
        ledSendValue = 1;
        send_queue_value(&ledSendValue);
        uint16_t raw = adc_read();  // take the raw value from 5th ADC channel
        float diodeVoltage = raw * CONVERSION_FACTOR;
        float temp = 27 - (diodeVoltage - 0.706) / 0.001721;  // Provided in the Pico datasheet
        write_temp_to_display(&display, &temp, &diodeVoltage);
        ledSendValue = 0;
        send_queue_value(&ledSendValue);
        vTaskDelay(200);
    }

    ssd1306_deinit(&display);
}

void led_flash_task(void *pvParameters) {
    bool is_connected = true;
    if (cyw43_arch_init()) {
        printf("WiFi init failed\n");
        is_connected = false;
    }

    uint ledReceiveValue = 0;  // LED Value to be received

    while (is_connected) {
        recieve_queue_value(&ledReceiveValue);
        if (ledReceiveValue == 1) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            printf("LED tuned ON!\n");
        } else {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            printf("LED turned OFF\n");
        }
    }
}

static void write_temp_to_display(ssd1306_t *disp, float *temp, float *voltage) {
    ssd1306_clear(disp);
    ssd1306_draw_string(disp, 40, 4, 2, "PICO");

    // temp data
    char temp_str[4];
    ftoa(*temp, temp_str, 1);
    char *temp_full = malloc(sizeof(char) * 20);  // Remember to free this after show
    if (temp != NULL) {
        strcpy(temp_full, "Temp: ");
        strcat(temp_full, temp_str);
        strcat(temp_full, " C");
        ssd1306_draw_string(disp, 13, 26, 1, temp_full);
    } else {
        ssd1306_draw_string(disp, 13, 26, 1, "NIL");
    }

    // voltage data
    char voltage_str[4];
    ftoa(*voltage, voltage_str, 2);
    char *volt_full = malloc(sizeof(char) * 20);  // Remember to free this after show
    if (temp != NULL) {
        strcpy(volt_full, "RP: ");
        strcat(volt_full, voltage_str);
        strcat(volt_full, "V");
        ssd1306_draw_string(disp, 13, 38, 1, volt_full);
    } else {
        ssd1306_draw_string(disp, 13, 38, 1, "NIL");
    }

    // ssd1306_draw_string(disp, 13, 32, 1, "C..");
    // ssd1306_draw_string(disp, 13, 42, 1, "Package Temp");
    ssd1306_draw_string(disp, 13, 52, 1, "RP2040 PACKAGE");
    ssd1306_show(disp);

    free(temp_full);  // temp mem freed here
    free(volt_full);  // volt mem freed here

    vTaskDelay(2000);
    ssd1306_poweroff(disp);
    vTaskDelay(200);
    ssd1306_poweron(disp);
}

static void setup_display_gpio() {
    i2c_init(i2c0, 400 * 1000);
    gpio_set_function(DISPLAY_SDA, GPIO_FUNC_I2C);
    gpio_set_function(DISPLAY_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(DISPLAY_SDA);
    gpio_pull_up(DISPLAY_SCL);
}

static void send_queue_value(uint *value) {
    // Sending ON data to the queue
    xQueueSend(
        led_queue,  // The queue you created
        value,      // Pointer for value to send to the queue
        0U          // Delay for sending to queue (0 for now since our queue should be empty)
    );
}

static void recieve_queue_value(uint *value) {
    // Receiving LED data from the queue
    xQueueReceive(
        led_queue,     // The queue you created
        value,         // Pointer for value to store the received data from the queue
        portMAX_DELAY  // Delay for sending to queue (we set max delay to wait for the value)
    );
}
