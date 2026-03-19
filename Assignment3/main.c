/*
 * main.c - ECE407 Assignment 3
 * INMP441 I2S digital microphone acquisition via RP2040 PIO
 *
 * I2S library adapted from:
 *   Daniel Collins, "rp2040_i2s_example", GitHub, 2022.
 *   https://github.com/malacalypse/rp2040_i2s_example
 *   Licensed under GPL-3.0
 *
 * Pin connections:
 *   INMP441 VDD  -> 3V3
 *   INMP441 GND  -> GND
 *   INMP441 L/R  -> GND  (mono, right channel)
 *   INMP441 WS   -> GP9  (LRCK, clock_pin_base + 1)
 *   INMP441 SCK  -> GP8  (BCK,  clock_pin_base)
 *   INMP441 SD   -> GP7  (DIN)
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "i2s.h"

// I2S config: 48kHz, 256x SCK, 32-bit depth
// sck_pin=10 (unused/unconnected), dout_pin=6 (unused), din_pin=7, clock_pin_base=8
// sck_enable=false: INMP441 has internal PLL, doesn't need master clock
static const i2s_config config = {
    .fs             = 48000,
    .sck_mult       = 256,
    .bit_depth      = 32,
    .sck_pin        = 10,   // not used
    .dout_pin       = 6,    // not used (no DAC)
    .din_pin        = 7,    // SD  -> GP7
    .clock_pin_base = 8,    // BCK -> GP8, LRCK -> GP9
    .sck_enable     = false
};

// Must be 8-byte aligned for DMA wrap to work (see i2s.h)
static __attribute__((aligned(8))) pio_i2s i2s;

// Flag set by DMA handler so main loop knows a buffer is ready to print
static volatile bool buffer_ready = false;
static volatile int32_t *ready_buffer = NULL;

// Taken from example code in referenced open-source I2S project
static void dma_i2s_in_handler(void) {
    // Determine which half of the double buffer was just filled, by checking the current DMA read address
    if (*(int32_t**)dma_hw->ch[i2s.dma_ch_in_ctrl].read_addr == i2s.input_buffer) {
        // DMA is now filling second half -> first half is ready
        ready_buffer = i2s.input_buffer;
    } else {
        // DMA is now filling first half -> second half is ready
        ready_buffer = &i2s.input_buffer[STEREO_BUFFER_SIZE];
    }
    buffer_ready = true;
    dma_hw->ints0 = 1u << i2s.dma_ch_in_data;  // clear IRQ
}

// Returns peak absolute value of left channel samples in the buffer
static int32_t find_peak(const int32_t *buf) {
    int32_t peak = 0;
    // Left channel at even indices, data is 24-bit MSB-aligned so shift right 8
    for (int i = 0; i < STEREO_BUFFER_SIZE; i += 2) {
        int32_t sample = buf[i] >> 8;  // 24-bit signed value
        if (sample < 0) sample = -sample;
        if (sample > peak) peak = sample;
    }
    return peak;
}

static void print_bar(int32_t peak) {
    // INMP441 max 24-bit value is 2^23 = 8,388,608
    // Scale to 40 characters wide
    const int32_t max_val = 1 << 23;
    const int bar_width = 40;
    int filled = (int)((int64_t)peak * bar_width / max_val);
    if (filled > bar_width) filled = bar_width;

    printf("\r|");
    for (int i = 0; i < bar_width; i++) {
        printf(i < filled ? "#" : " ");
    }
    printf("| %-8ld", peak);
    stdio_flush();
}


int main() {
    // 132 MHz gives clean integer dividers for 48kHz I2S at 32-bit depth
    set_sys_clock_khz(132000, true);
    stdio_init_all();

    // Small delay to let USB serial connect before printing
    sleep_ms(2000);
    printf("ECE407 Assignment 3 - INMP441 I2S Audio Capture\n");
    printf("System clock: %lu Hz\n", clock_get_hz(clk_sys));
    printf("Sample rate:  48000 Hz, 32-bit frames\n\n");

    i2s_program_start_synched(pio0, &config, dma_i2s_in_handler, &i2s);

    printf("I2S started. Listening...\n");

    while (true) {
        if (buffer_ready) {
            buffer_ready = false;
            const int32_t *buf = (const int32_t *)ready_buffer;
            int32_t peak = find_peak(buf);
            print_bar(peak);
        }
    }

    return 0;
}
