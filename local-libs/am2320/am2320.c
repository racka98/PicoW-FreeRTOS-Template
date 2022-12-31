
#include "am2320.h"

/**************************************************************************/
/*!
    @brief  read the temperature from the device
    @return the temperature reading as a floating point value
*/
/**************************************************************************/
void read_temperature(float *pTemp);

/**************************************************************************/
/*!
    @brief  read the humidity from the device
    @return the humidity reading as a floating point value
*/
/**************************************************************************/
void read_humidity(float *pHum);

static uint16_t readRegister16(uint8_t reg);
static uint16_t crc16(uint8_t *buffer, uint8_t nbytes);

static void sensor_read() {
    sleep_ms(3000);
    am2320_data values;
    while (true) {
        printf("Reading Values... \n");
        values = am2320_read_data();
        printf("Temp: %f C\n", values.temp);
        printf("Humidity: %f%%\n", values.hum);
        sleep_ms(5000);
    }
}

void test_temp_sensor() {
    // Configure the I2C
    printf("Configuring I2C\n");
    i2c_init(DEFAULT_I2C_PORT, 100 * 1000);
    gpio_set_function(DEFAULT_SDA, GPIO_FUNC_I2C);
    gpio_set_function(DEFAULT_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(DEFAULT_SDA);
    gpio_pull_up(DEFAULT_SCL);

    // initialize
    sensor_read();
}

am2320_data am2320_read_data() {
    // Create Empty sensor data
    am2320_data data = {.temp = -40.0, .hum = 0.0};

    // use an 8 byte buffer for sending and receiving data
    uint8_t buffer[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // write to the dht to wake it up
    i2c_write_blocking(DEFAULT_I2C_PORT, AM2320_ADDRESS, buffer, 1, false);
    sleep_ms(10);

    // send request to read all data from the dht (4 bytes of temp and hum data)
    buffer[0] = AM2320_CMD_READREG;
    buffer[1] = AM2320_START_ADDRESS;
    buffer[2] = AM2320_DATA_END_ADDRESS;
    uint8_t write_buff[3] = {AM2320_CMD_READREG, AM2320_START_ADDRESS, AM2320_DATA_END_ADDRESS};
    printf("Write data: ");
    printf("[0x%02x 0x%02x 0x%02x]\n", write_buff[0], write_buff[1], write_buff[2]);
    int test;
    test = i2c_write_blocking(DEFAULT_I2C_PORT, AM2320_ADDRESS, write_buff, 3, true);
    sleep_ms(5);
    if (test == PICO_ERROR_GENERIC) {
        printf("A problem with the device!\n");
    } else {
        printf("Bytes written: %d\n", test);
    }

    // dht sends us back 8 bytes, read them
    /*
     * Read out 8 bytes of data
     * Byte 0: Should be Modbus function code 0x03
     * Byte 1: Should be number of registers to read (0x04)
     * Byte 2: Humidity msb
     * Byte 3: Humidity lsb
     * Byte 4: Temperature msb
     * Byte 5: Temperature lsb
     * Byte 6: CRC lsb byte
     * Byte 7: CRC msb byte
     */
    test = i2c_read_blocking(DEFAULT_I2C_PORT, AM2320_ADDRESS, buffer, 8, false);
    if (test == PICO_ERROR_GENERIC) {
        printf("A problem with the device on READ!\n");
    } else {
        printf("Bytes read: %d\n", test);
    }

    // if echoed command was not to read 4 bytes of data registers, error
    printf("Is read addr correct: %d\n", buffer[0] == AM2320_ADDRESS || buffer[0] == _u(0x03));
    printf("[0x%02x 0x%02x  0x%02x 0x%02x  0x%02x 0x%02x  0x%02x 0x%02x]\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);

    if (buffer[0] != AM2320_CMD_READREG || buffer[1] != AM2320_DATA_END_ADDRESS) {
        printf("Invalid data read!\n");
        return data;
    }

    // calulate checksum and compare vs recived checksum (2 last bytes)
    // if no match, error
    uint16_t the_crc = (buffer[7] << 8) | buffer[6];
    uint16_t calc_crc = crc16(buffer, 6);
    if (the_crc != calc_crc) {
        printf("CRC Checksum no match!\n");
        return data;
    }

    // extract the temp and hum data from recived data
    // temp: high is byte 4, low is byte 5
    // hum: high is byte 2, low is byte 3
    uint16_t tempData = (buffer[4] << 8) | buffer[5];
    uint16_t humData = (buffer[2] << 8) | buffer[3];

    // fill struct with temp and hum data by dividing recived values by 10,
    // clearing high of temp as it only communicates sign data
    data.temp = (tempData & 0x7FFF) / 10;
    data.hum = humData / 10;

    // if first bit of temp is set, it is negative
    if (tempData & 0x8000) {
        data.temp *= -1;
    }

    return data;
}

void read_temperature(float *pTemp) {
    uint16_t t = readRegister16(AM2320_REG_TEMP_H);
    if (t == 0xFFFF)
        *pTemp = -40;
    return;  // End execution
    // check sign bit - the temperature MSB is signed , bit 0-15 are magnitude
    if (t & 0x8000) {
        *pTemp = -(int16_t)(t & 0x7fff);
    } else {
        *pTemp = (int16_t)t;
    }
    *pTemp = *pTemp / 10.0;
}

void read_humidity(float *pHum) {
    uint16_t h = readRegister16(AM2320_REG_HUM_H);
    if (h == 0xFFFF) {
        *pHum = 0;
        return;  // End Execution
    }

    *pHum = h / 10.0;
}

/**************************************************************************/
/*!
    @brief  read 2 bytes from a hardware register
    @param reg the register to read
    @return the read value as a 2 byte unsigned integer
*/
/**************************************************************************/
static uint16_t readRegister16(uint8_t reg) {
    uint8_t buffer[6] = {0, 0, 0, 0, 0, 0};

    // wake up
    // i2c_dev->write(buffer, 1); - From Arduino Adafruit
    i2c_write_blocking(DEFAULT_I2C_PORT, AM2320_ADDRESS, buffer, 1, false);
    sleep_ms(10);  // wait 10 ms
    printf("Buffer Wake: %s\n", buffer);

    // send a command to read register
    buffer[0] = AM2320_CMD_READREG;
    buffer[1] = reg;
    buffer[2] = 2;  // 2 bytes
    // i2c_dev->write(buffer, 3); - From Arduino Adafruit
    i2c_write_blocking(DEFAULT_I2C_PORT, AM2320_ADDRESS, buffer, 3, true);
    printf("Buffer Request Write 0: %c\n", buffer[0]);
    printf("Buffer Request Write 1: %c\n", buffer[1]);
    sleep_ms(2);  // wait 2 ms

    // 2 bytes preamble, 2 bytes data, 2 bytes CRC
    i2c_read_blocking(DEFAULT_I2C_PORT, AM2320_ADDRESS, buffer, 6, false);
    printf("Buffer Read 0: %c\n", buffer[0]);
    printf("Buffer Read 1: %c\n", buffer[1]);
    // i2c_dev->read(buffer, 6); - From Arduino Adafruit

    printf("Is read addr correct: %d\n", buffer[0] == AM2320_ADDRESS || buffer[0] == _u(0x03));
    printf("[0x%02x 0x%02x  0x%02x 0x%02x  0x%02x 0x%02x  0x%02x 0x%02x]\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);

    if (buffer[0] != 0x03)
        return 0xFFFF;  // must be 0x03 modbus reply
    if (buffer[1] != 2)
        return 0xFFFF;  // must be 2 bytes reply

    uint16_t the_crc = buffer[5];
    the_crc <<= 8;
    the_crc |= buffer[4];
    uint16_t calc_crc = crc16(buffer, 4);  // preamble + data
    if (the_crc != calc_crc)
        return 0xFFFF;

    // All good!
    uint16_t ret = buffer[2];
    ret <<= 8;
    ret |= buffer[3];

    return ret;
}

/**************************************************************************/
/*!
    @brief  perfor a CRC check to verify data
    @param buffer the pointer to the data to check
    @param nbytes the number of bytes to calculate the CRC over
    @return the calculated CRC
*/
/**************************************************************************/
static uint16_t crc16(uint8_t *buffer, uint8_t nbytes) {
    unsigned short crc = 0xFFFF;
    uint8_t s = 0x00;
    while (nbytes--) {
        crc ^= *buffer++;
        for (s = 0; s < 8; s++) {
            if ((crc & 0x01) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else
                crc >>= 1;
        }
    }
    return crc;
}
