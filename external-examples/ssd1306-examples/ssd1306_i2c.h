#pragma once

#include <stdio.h>
#include "hardware/i2c.h"

/* Example code to talk to a SSD1306 OLED display, 128 x 64 pixels
   NOTE: Ensure the device is capable of being driven at 3.3v NOT 5v. The Pico
   GPIO (and therefore I2C) cannot be used at 5v.
   Connections on Raspberry Pi Pico board, other boards may vary.
   GPIO 12 (pin 16)-> SDA on SSD1306 board
   GPIO 13 (pin 17)-> SCL on SSD1306 board
   3.3v (pin 36) -> VCC on SSD1306 board
   GND (pin 38)  -> GND on SSD1306 board
*/

// By default these devices are on bus address 0x3C or 0x3D. Check your documentation.
#define DEVICE_ADDRESS 0x3C

#define I2C_PORT i2c0
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

// This can be overclocked, 2000 seems to work on the device being tested
// Spec says 400 is the maximum. Try faster clocks until it stops working!
// KHz.
#define I2C_CLOCK 400

#define SSD1306_LCDWIDTH 128
#define SSD1306_LCDHEIGHT 64
#define SSD1306_FRAMEBUFFER_SIZE (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8)

// Not currently used.
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR 0x22
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_ACTIVATE_SCROLL 0x2F

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_SEGREMAP0 0xA0
#define SSD1306_SEGREMAP127 0xA1
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB

/// @brief  Display setup data
typedef struct DisplaySetup {
    uint SDA_PIN;
    uint SCL_PIN;
    i2c_inst_t *I2C_INSTANCE;
} DisplaySetup;

/// @brief Initialize the Display for use
void ssd1306_display_init(DisplaySetup *setup);

/**
 * Invert the display color
*/
void ssd1306_invert_display(bool yes);

/**
 * Clears everything on the display (clear the framebuffer)
*/
void ssd1306_clear_display();

/**
 * Call this is update your display after writting data to it.
 * This copies the entire framebuffer to the display.
*/
void ssd1306_update_display();

/**
 * Draw a line as specified start and end coordinates
 * Basic Bresenhams.
 * @attention Both x & y must be less than SSD1306_LCDWIDTH & SSD1306_LCDHEIGHT respectively
 * @attention x range = 0..(SSD1306_LCDWIDTH - 1)
 * @attention y range = 0..(SSD1306_LCDHEIGHT - 1)
 * @param startX Starting point of pixel on X-axis
 * @param startY Starting point of pixel on Y-axis
 * @param startX End point of pixel on X-axis
 * @param startX End point of pixel on Y-axis
*/
void ssd1306_draw_line1(int startX, int startY, int endX, int endY, bool on);

/**
 * Write a character to the display.
 * @attention Both x & y must be less than SSD1306_LCDWIDTH & SSD1306_LCDHEIGHT respectively
 * @attention x range = 0..(SSD1306_LCDWIDTH - 1)
 * @attention y range = 0..(SSD1306_LCDHEIGHT - 1)
 * @param startX Starting point of pixel on X-axis
 * @param startY Starting point of pixel on Y-axis
*/
void ssd1306_write_char(uint startX, uint startY, char ch);

/**
 * Write a big sized character to the display.
 * @attention Both x & y must be less than SSD1306_LCDWIDTH & SSD1306_LCDHEIGHT respectively
 * @attention x range = 0..(SSD1306_LCDWIDTH - 1)
 * @attention y range = 0..(SSD1306_LCDHEIGHT - 1)
 * @param startX Starting point of pixel on X-axis
 * @param startY Starting point of pixel on Y-axis
*/
void ssd1306_write_big_char(uint startX, uint startY, char ch);

/**
 * Write a whole string to the display
 * @attention Both x & y must be less than SSD1306_LCDWIDTH & SSD1306_LCDHEIGHT respectively
 * @attention x range = 0..(SSD1306_LCDWIDTH - 1)
 * @attention y range = 0..(SSD1306_LCDHEIGHT - 1)
 * @param startX Starting point of pixel on X-axis
 * @param startY Starting point of pixel on Y-axis
*/
void ssd1306_write_string(int startX, int startY, char *str);

/**
 * Write a big sized whole string to the display
 * @attention Both x & y must be less than SSD1306_LCDWIDTH & SSD1306_LCDHEIGHT respectively
 * @attention x range = 0..(SSD1306_LCDWIDTH - 1)
 * @attention y range = 0..(SSD1306_LCDHEIGHT - 1)
 * @param startX Starting point of pixel on X-axis
 * @param startY Starting point of pixel on Y-axis
*/
void ssd1306_write_big_string(int startX, int startY, char *str);

/**
 * Run to test your display
*/
void ssd1306_display_test(void);
