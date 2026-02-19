/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

#define PIN_MISO 4
#define PIN_CS   5
#define PIN_SCK  2
#define PIN_MOSI 3

#define SPI_PORT spi0
#define READ_BIT 0x80
#define MULTI_BIT 0x40 // ADXL343 requires bit 6 to be set for multi-byte reads/writes

static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

// Helper function to write a single register
static void write_register(uint8_t reg, uint8_t data) {
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = data;
    cs_select();
    spi_write_blocking(SPI_PORT, buf, 2);
    cs_deselect();
    sleep_ms(1);
}

static void read_registers(uint8_t reg, uint8_t *buf, uint16_t len) {
    // Set the read bit
    reg |= READ_BIT;
    
    // If reading more than 1 byte, set the multi-byte bit for ADXL343
    if (len > 1) {
        reg |= MULTI_BIT;
    }

    cs_select();
    spi_write_blocking(SPI_PORT, &reg, 1);
    spi_read_blocking(SPI_PORT, 0, buf, len);
    cs_deselect();
    sleep_ms(1);
}

static void adxl343_init() {
    // Set FIFO mode to STREAM (Reg 0x38, Val 0x80)
    write_register(0x38, 0x80);

    // Set DATA_FORMAT register to full resolution, +/- 16g (Reg 0x31, Val 0x0B)
    write_register(0x31, 0b00001011);

    // Set POWER_CTL register to measure mode (Reg 0x2D, Val 0x08)
    write_register(0x2D, 0x08);
}

static void adxl343_read_raw(int16_t accel[3]) {
    uint8_t buffer[6];

    // Read 6 bytes starting from DATAX0 (0x32)
    read_registers(0x32, buffer, 6);

    // ADXL343 is Little Endian (unlike the MPU9250), so we shift the odd bytes
    accel[0] = (int16_t)((buffer[1] << 8) | buffer[0]); // X
    accel[1] = (int16_t)((buffer[3] << 8) | buffer[2]); // Y
    accel[2] = (int16_t)((buffer[5] << 8) | buffer[4]); // Z
}

int main() {
    stdio_init_all();

    printf("Hello, ADXL343! Reading raw data from registers via SPI...\n");

    // Initialize SPI0 at 500kHz
    spi_init(SPI_PORT, 500 * 1000);

    //Set SPI format
    spi_set_format(SPI_PORT, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    bi_decl(bi_3pins_with_func(PIN_MISO, PIN_MOSI, PIN_SCK, GPIO_FUNC_SPI));

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    bi_decl(bi_1pin_with_name(PIN_CS, "SPI CS"));

    // Check Device ID. ADXL343 standard ID is 0xE5 (229)
    uint8_t id;
    read_registers(0x00, &id, 1);
    printf("Device ID: 0x%02x\n", id);

    // Configure the sensor
    adxl343_init();

    int16_t acceleration[3];

    while (1) {
        adxl343_read_raw(acceleration);

        // Scale factor for ADXL343 in full resolution mode is 256 LSB/g
        float accel_g[3];
        accel_g[0] = acceleration[0] / 256.0f;
        accel_g[1] = acceleration[1] / 256.0f;
        accel_g[2] = acceleration[2] / 256.0f;

        // Print both raw and scaled values
        printf("\rAcc. X = %6d (%.3f g), Y = %6d (%.3f g), Z = %6d (%.3f g)\n",
               acceleration[0], accel_g[0],
               acceleration[1], accel_g[1],
               acceleration[2], accel_g[2]);

        sleep_ms(100);
    }
}