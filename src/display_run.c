#include <hardware/i2c.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ssd1306.h"

const uint8_t num_chars_per_disp[] = {7, 7, 7, 5};

#define SLEEPTIME 25
#define DISPLAY_SDA 4
#define DISPLAY_SCL 5

static void setup_gpios(void);
static void lets_go(ssd1306_t *disp);

void run_display(void) {
    printf("Configuring GPIO PINS\n");
    setup_gpios();
    ssd1306_t display;
    display.external_vcc = false;
    ssd1306_init(&display, 128, 64, 0x3C, i2c0);
    while (true) {
        printf("Running display...\n");
        lets_go(&display);
    }
    ssd1306_deinit(&display);
}

static void setup_gpios(void) {
    i2c_init(i2c0, 400 * 1000);
    gpio_set_function(DISPLAY_SDA, GPIO_FUNC_I2C);
    gpio_set_function(DISPLAY_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(DISPLAY_SDA);
    gpio_pull_up(DISPLAY_SCL);
}

static void lets_go(ssd1306_t *disp) {
    ssd1306_clear(disp);
    ssd1306_draw_string(disp, 13, 2, 2, "RACKADEV");
    ssd1306_draw_string(disp, 13, 22, 1, "Kotlin..");
    ssd1306_draw_string(disp, 13, 32, 1, "C..");
    ssd1306_draw_string(disp, 13, 42, 1, "Java..");
    ssd1306_draw_string(disp, 13, 52, 1, "Rust..");
    ssd1306_show(disp);
    sleep_ms(2000);
    ssd1306_poweroff(disp);
    sleep_ms(300);
    ssd1306_poweron(disp);
}
