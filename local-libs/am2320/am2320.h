#pragma once

#include <hardware/i2c.h>
#include <pico/stdlib.h>
#include <stdio.h>

#define DEFAULT_SDA 10         // GPIO 10
#define DEFAULT_SCL 11         // GPIO 11
#define DEFAULT_I2C_PORT i2c1  // i2c1

#define AM2320_ADDRESS _u(0x5C)           // < Address of sensor
#define AM2320_START_ADDRESS _u(0x00)     // < start address of the data
#define AM2320_DATA_END_ADDRESS _u(0x04)  // < end address of the temp & humidity data
#define AM2320_CMD_READREG _u(0x03)       // < read register command
#define AM2320_REG_TEMP_H _u(0x02)        // < high temp register address
#define AM2320_REG_HUM_H _u(0x00)         // < high humidity register address

typedef struct {
    uint8_t address;   /**< i2c address of display*/
    i2c_inst_t *i2c_i; /**< i2c connection instance */
} am2320_t;

typedef struct {
    float temp;  // < temperature reading from the sensor in °C
    float hum;   // < humidity reading from the sensor
} am2320_data;

void test_temp_sensor();

am2320_data am2320_read_data();
