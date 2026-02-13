/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

/* Example code to talk to a ADXL343 MEMS accelerometer and gyroscope

   This is taking to simple approach of simply reading registers. It's perfectly
   possible to link up an interrupt line and set things up to read from the
   inbuilt FIFO to make it more useful.

   NOTE: Ensure the device is capable of being driven at 3.3v NOT 5v. The Pico
   GPIO (and therefore I2C) cannot be used at 5v.

   You will need to use a level shifter on the I2C lines if you want to run the
   board at 5v.

   Connections on Raspberry Pi Pico board, other boards may vary.

   GPIO PICO_DEFAULT_I2C_SDA_PIN (On Pico this is GP4 (pin 6)) -> SDA on ADXL343 board
   GPIO PICO_DEFAULT_I2C_SCL_PIN (On Pico this is GP5 (pin 7)) -> SCL on ADXL343 board
   3.3v (pin 36) -> VCC on ADXL343 board
   GND (pin 38)  -> GND on ADXL343 board
*/

// ADXL343 alternate I2C address
static int addr = 0x53;

#ifdef i2c_default
static void adxl343_reset() {
    // Print device Id
    uint8_t device_id_reg = 0x00;
    i2c_write_blocking(i2c_default, addr, &device_id_reg, 1, true); // true to keep master control of bus
    uint8_t device_id;
    i2c_read_blocking(i2c_default, addr, &device_id, 1, false); // false to release bus after read
    printf("Device ID: 0x%02x\n", device_id);

    // Set FIFO mode to STREAM
    uint8_t fifo_ctl_reg = 0x38;
    uint8_t fifo_ctl_val = 0x80; // STREAM mode
    uint8_t fifo_ctl_buf[2] = {fifo_ctl_reg, fifo_ctl_val};
    i2c_write_blocking(i2c_default, addr, fifo_ctl_buf, 2, false);

    // Set DATA_FORMAT register to full resolution, +/- 16g
    uint8_t data_format_reg = 0x31;
    uint8_t data_format_val = 0b00001011; // Full resolution, +/- 16g
    uint8_t data_format_buf[2] = {data_format_reg, data_format_val};
    i2c_write_blocking(i2c_default, addr, data_format_buf, 2, false);

    // Set POWER_CTL register to measure mode
    uint8_t power_ctl_reg = 0x2D;
    uint8_t power_ctl_val = 0b00001000; // Measure mode
    uint8_t power_ctl_buf[2] = {power_ctl_reg, power_ctl_val};
    i2c_write_blocking(i2c_default, addr, power_ctl_buf, 2, false);
}

static void adxl343_read_raw(int16_t accel[3]) {
    // For this particular device, we send the device the register we want to read
    // first, then subsequently read from the device. The register is auto incrementing
    // so we don't need to keep sending the register we want, just the first.

    uint8_t buffer[6];

    uint8_t start_reg = 0x32; // DATAX0 register
    i2c_write_blocking(i2c_default, addr, &start_reg, 1, true); // true to keep master control of bus
    i2c_read_blocking(i2c_default, addr, buffer, 6, false); // false to release bus after read

    // Combine bytes into int16_t values
    accel[0] = (int16_t)((buffer[1] << 8) | buffer[0]); // X
    accel[1] = (int16_t)((buffer[3] << 8) | buffer[2]); // Y
    accel[2] = (int16_t)((buffer[5] << 8) | buffer[4]); // Z
}
#endif

int main() {
    stdio_init_all();
#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
    #warning i2c/adxl343_i2c example requires a board with I2C pins
    puts("Default I2C pins were not defined");
    return 0;
#else
    printf("Hello, ADXL343! Reading raw data from registers...\n");

    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    adxl343_reset();

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
#endif
}
