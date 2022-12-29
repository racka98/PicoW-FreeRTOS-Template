/**
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ssd1306_i2c.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "ssd1306_font.h"

// We need a 0x40 in the byte before our framebuffer
uint8_t _Framebuffer[SSD1306_FRAMEBUFFER_SIZE + 1] = {0x40};
uint8_t *Framebuffer = _Framebuffer + 1;

static void SendCommand(uint8_t cmd) {
    uint8_t buf[] = {0x00, cmd};
    i2c_write_blocking(I2C_PORT, DEVICE_ADDRESS, buf, 2, false);
}

static void SendCommandBuffer(uint8_t *inbuf, int len) {
    i2c_write_blocking(I2C_PORT, DEVICE_ADDRESS, inbuf, len, false);
}

static void SSD1306_initialise() {
    uint8_t init_cmds[] =
        {0x00,
         SSD1306_DISPLAYOFF,
         SSD1306_SETMULTIPLEX, 0x3f,
         SSD1306_SETDISPLAYOFFSET, 0x00,
         SSD1306_SETSTARTLINE,
         SSD1306_SEGREMAP127,
         SSD1306_COMSCANDEC,
         SSD1306_SETCOMPINS, 0x12,
         SSD1306_SETCONTRAST, 0xff,
         SSD1306_DISPLAYALLON_RESUME,
         SSD1306_NORMALDISPLAY,
         SSD1306_SETDISPLAYCLOCKDIV, 0x80,
         SSD1306_CHARGEPUMP, 0x14,
         SSD1306_DISPLAYON,
         SSD1306_MEMORYMODE, 0x00,                     // 0 = horizontal, 1 = vertical, 2 = page
         SSD1306_COLUMNADDR, 0, SSD1306_LCDWIDTH - 1,  // Set the screen wrapping points
         SSD1306_PAGEADDR, 0, 7};

    SendCommandBuffer(init_cmds, sizeof(init_cmds));
}

void ssd1306_display_init(DisplaySetup *setup) {
    i2c_init(setup->I2C_INSTANCE, I2C_CLOCK * 1000);
    gpio_set_function(setup->SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(setup->SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(setup->SDA_PIN);
    gpio_pull_up(setup->SCL_PIN);

    SSD1306_initialise();
}

void ssd1306_invert_display(bool yes) {
    if (yes)
        SendCommand(SSD1306_INVERTDISPLAY);
    else
        SendCommand(SSD1306_NORMALDISPLAY);
}

// This copies the entire framebuffer to the display.
void ssd1306_update_display() {
    i2c_write_blocking(I2C_PORT, DEVICE_ADDRESS, _Framebuffer, sizeof(_Framebuffer), false);
}

void ssd1306_clear_display() {
    memset(Framebuffer, 0, SSD1306_FRAMEBUFFER_SIZE);
    ssd1306_update_display();
}

static void SetPixel(int pixelX, int pixelY, bool on) {
    assert(pixelX >= 0 && pixelX < SSD1306_LCDWIDTH && pixelY >= 0 && pixelY < SSD1306_LCDHEIGHT);

    // The calculation to determine the correct bit to set depends on which address
    // mode we are in. This code assumes horizontal

    // The video ram on the SSD1306 is split up in to 8 rows, one bit per pixel.
    // Each row is 128 long by 8 pixels high, each byte vertically arranged, so byte 0 is pixelX=0, pixelY=0->7,
    // byte 1 is pixelX = 1, pixelY=0->7 etc

    // This code could be optimised, but is like this for clarity. The compiler
    // should do a half decent job optimising it anyway.

    const int BytesPerRow = 128;  // 128 pixels, 1bpp, but each row is 8 pixel high, so (128 / 8) * 8

    int byte_idx = (pixelY / 8) * BytesPerRow + pixelX;
    uint8_t byte = Framebuffer[byte_idx];

    if (on)
        byte |= 1 << (pixelY % 8);
    else
        byte &= ~(1 << (pixelY % 8));

    Framebuffer[byte_idx] = byte;
}

void ssd1306_draw_line1(int startX, int startY, int endX, int endY, bool on) {
    int dx = abs(endX - startX);
    int sx = startX < endX ? 1 : -1;
    int dy = -abs(endY - startY);
    int sy = startY < endY ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (true) {
        SetPixel(startX, startY, on);

        if (startX == endX && startY == endY)
            break;
        e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            startX += sx;
        }
        if (e2 <= dx) {
            err += dx;
            startY += sy;
        }
    }
}

static uint8_t reverse(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

static inline int GetFontIndex(uint8_t ch) {
    if (ch >= 'A' && ch <= 'Z')
        return ch - 'A' + 1;
    else if (ch >= '0' && ch <= '9')
        return ch - '0' + 27;
    else
        return 0;  // Not got that char so space.
}

static uint8_t reversed[sizeof(font)] = {0};

static void FillReversedCache() {
    // calculate and cache a reversed version of fhe font, because I defined it upside down...doh!
    for (int i = 0; i < sizeof(font); i++)
        reversed[i] = reverse(font[i]);
}

void ssd1306_write_char(uint startX, uint startY, char ch) {
    if (reversed[0] == 0)
        FillReversedCache();

    if (startX > SSD1306_LCDWIDTH - 8 || startY > SSD1306_LCDHEIGHT - 8)
        return;

    // For the moment, only write on Y row boundaries (every 8 vertical pixels)
    startY = startY / 8;

    ch = toupper(ch);
    int idx = GetFontIndex(ch);
    int fb_idx = startY * 128 + startX;

    for (int i = 0; i < 8; i++) {
        Framebuffer[fb_idx++] = reversed[idx * 8 + i];
    }
}

static uint16_t ExpandByte(uint8_t b) {
    uint16_t w = 0;
    for (int i = 7; i >= 0; i--) {
        uint16_t t = (b & (1 << i));
        w |= (t << i);
        w |= (t << (i + 1));
    }
    return w;
}

void ssd1306_write_big_char(uint startX, uint startY, char ch) {
    if (reversed[0] == 0)
        FillReversedCache();

    if (startX > SSD1306_LCDWIDTH - 16 || startY > SSD1306_LCDHEIGHT - 16)
        return;

    // For the moment, only write on Y row boundaries (every 8 vertical pixels)
    startY = startY / 8;

    ch = toupper(ch);
    int idx = GetFontIndex(ch);
    int fb_idx = startY * 128 + startX;

    for (int i = 0; i < 8; i++) {
        uint16_t w = ExpandByte(reversed[idx * 8 + i]);
        Framebuffer[fb_idx] = w & 0x0ff;
        Framebuffer[fb_idx + 1] = w & 0x0ff;
        Framebuffer[fb_idx + 128] = w >> 8;
        Framebuffer[fb_idx + 129] = w >> 8;
        fb_idx += 2;
    }
}

void ssd1306_write_string(int startX, int startY, char *str) {
    // Cull out any string off the screen
    if (startX > SSD1306_LCDWIDTH - 8 || startY > SSD1306_LCDHEIGHT - 8)
        return;

    while (*str) {
        ssd1306_write_char(startX, startY, *str++);
        startX += 8;
    }
}

void ssd1306_write_big_string(int startX, int startY, char *str) {
    // Cull out any string off the screen
    if (startX > SSD1306_LCDWIDTH - 16 || startY > SSD1306_LCDHEIGHT - 16)
        return;

    while (*str) {
        ssd1306_write_big_char(startX, startY, *str++);
        startX += 16;
    }
}

void ssd1306_display_test(void) {
    // stdio_init_all();

    printf("Hello, SSD1306! ...\n");

    static DisplaySetup setup = { SDA_PIN: I2C_SDA_PIN, SCL_PIN: I2C_SCL_PIN, I2C_INSTANCE: I2C_PORT };
    ssd1306_display_init(&setup);

    while (1) {
        ssd1306_clear_display();

        ssd1306_draw_line1(0, 0, SSD1306_LCDWIDTH - 1, 0, true);
        ssd1306_draw_line1(0, 0, 0, SSD1306_LCDHEIGHT - 1, true);
        ssd1306_draw_line1(SSD1306_LCDWIDTH - 1, 0, SSD1306_LCDWIDTH - 1, SSD1306_LCDHEIGHT - 1, true);
        ssd1306_draw_line1(0, SSD1306_LCDHEIGHT - 1, SSD1306_LCDWIDTH - 1, SSD1306_LCDHEIGHT - 1, true);

        ssd1306_write_string(13, 8, "HELLO SSD1306");

        for (int ch = '9'; ch >= '1'; ch--) {
            ssd1306_write_big_char(56, 36, ch);
            ssd1306_update_display();
            sleep_ms(500);
        }

        ssd1306_write_big_string(0, 35, "BLASTOFF");
        ssd1306_update_display();

        for (int i = 0; i < 10; i++) {
            ssd1306_invert_display(true);
            sleep_ms(100);
            ssd1306_invert_display(false);
            sleep_ms(100);
        }

        for (int i = 0; i < SSD1306_LCDWIDTH; i++) {
            if (i >= SSD1306_LCDWIDTH / 2) {
                ssd1306_draw_line1(i, 0, i, SSD1306_LCDHEIGHT - 1, false);
                ssd1306_draw_line1(SSD1306_LCDWIDTH - i - 1, 0, SSD1306_LCDWIDTH - i - 1, SSD1306_LCDHEIGHT - 1, false);
            } else {
                ssd1306_draw_line1(i, 0, i, SSD1306_LCDHEIGHT - 1, true);
                ssd1306_draw_line1(SSD1306_LCDWIDTH - i - 1, 0, SSD1306_LCDWIDTH - i - 1, SSD1306_LCDHEIGHT - 1, true);
            }
            ssd1306_update_display();
        }
    }

    printf("Done\n");

    // return 0;
}
