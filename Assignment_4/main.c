/*
 * main.c - ECE407 Assignment_4
 * INMP441 I2S digital microphone acquisition via RP2040 PIO
 *
 * I2S library adapted from:
 * Daniel Collins, "rp2040_i2s_example", GitHub, 2022.
 * https://github.com/malacalypse/rp2040_i2s_example
 * Licensed under GPL-3.0
 *
 * Pin connections:
 * INMP441 VDD  -> 3V3
 * INMP441 GND  -> GND
 * INMP441 L/R  -> GND  (mono, right channel)
 * INMP441 WS   -> GP9  (LRCK, clock_pin_base + 1)
 * INMP441 SCK  -> GP8  (BCK,  clock_pin_base)
 * INMP441 SD   -> GP7  (DIN)
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "i2s.h"

// I2S config: 48kHz, 256x SCK, 32-bit depth
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

static __attribute__((aligned(8))) pio_i2s i2s;
static volatile bool buffer_ready = false;
static volatile int32_t *ready_buffer = NULL;

static void dma_i2s_in_handler(void) {
    if (*(int32_t**)dma_hw->ch[i2s.dma_ch_in_ctrl].read_addr == i2s.input_buffer) {
        ready_buffer = i2s.input_buffer;
    } else {
        ready_buffer = &i2s.input_buffer[STEREO_BUFFER_SIZE];
    }
    buffer_ready = true;
    dma_hw->ints0 = 1u << i2s.dma_ch_in_data; 
}

#define NUM_TAPS 101

const float filter_taps[NUM_TAPS] = {
    -0.0006147744f, -0.0006275957f, -0.0006519365f, -0.0006876197f, -0.0007340555f,
    -0.0007902485f, -0.0008548121f, -0.0009259897f, -0.0010016854f, -0.0010794995f,
    -0.0011567720f, -0.0012306307f, -0.0012980448f, -0.0013558817f, -0.0014009676f,
    -0.0014301491f, -0.0014403562f, -0.0014286646f, -0.0013923568f, -0.0013289803f,
    -0.0012364027f, -0.0011128617f, -0.0009570098f, -0.0007679526f, -0.0005452799f,
    -0.0002890890f,  0.0000000000f,  0.0003208379f,  0.0006717486f,  0.0010505401f,
     0.0014545246f,  0.0018805469f,  0.0023250207f,  0.0027839733f,  0.0032530963f,
     0.0037278027f,  0.0042032900f,  0.0046746063f,  0.0051367205f,  0.0055845945f,
     0.0060132557f,  0.0064178700f,  0.0067938127f,  0.0071367373f,  0.0074426397f,
     0.0077079187f,  0.0079294299f,  0.0081045330f,  0.0082311318f,  0.0083077058f,
     0.0083333333f,  0.0083077058f,  0.0082311318f,  0.0081045330f,  0.0079294299f,
     0.0077079187f,  0.0074426397f,  0.0071367373f,  0.0067938127f,  0.0064178700f,
     0.0060132557f,  0.0055845945f,  0.0051367205f,  0.0046746063f,  0.0042032900f,
     0.0037278027f,  0.0032530963f,  0.0027839733f,  0.0023250207f,  0.0018805469f,
     0.0014545246f,  0.0010505401f,  0.0006717486f,  0.0003208379f,  0.0000000000f,
    -0.0002890890f, -0.0005452799f, -0.0007679526f, -0.0009570098f, -0.0011128617f,
    -0.0012364027f, -0.0013289803f, -0.0013923568f, -0.0014286646f, -0.0014403562f,
    -0.0014301491f, -0.0014009676f, -0.0013558817f, -0.0012980448f, -0.0012306307f,
    -0.0011567720f, -0.0010794995f, -0.0010016854f, -0.0009259897f, -0.0008548121f,
    -0.0007902485f, -0.0007340555f, -0.0006876197f, -0.0006519365f, -0.0006275957f,
    -0.0006147744f,
};

static float history[NUM_TAPS] = {0};
static int history_idx = 0;

// NEW DEBUG LOGIC: 加入正規化與去除直流偏移 (DC Offset)
static float get_500hz_energy(const int32_t *buf) {
    float total_energy = 0.0f;
    
    // 注意：這裡必須加上 static，讓它在每次讀取 buffer 時能記住前一次的 DC 值
    static float dc_offset = 0.0f; 
    
    for (int i = 0; i < STEREO_BUFFER_SIZE; i += 2) {
        // 1. 正規化：將 24-bit 整數縮小到 -1.0 到 1.0 之間 (2^23 = 8388608)
        float raw_sample = (float)(buf[i] >> 8) / 8388608.0f; 
        
        // 2. 簡易去除 DC offset (這是一個簡易的 IIR 高通濾波器)
        dc_offset = 0.999f * dc_offset + 0.001f * raw_sample;
        float sample = raw_sample - dc_offset;
        
        history[history_idx] = sample;
        
        float filtered_out = 0.0f;
        for (int k = 0; k < NUM_TAPS; k++) {
            int idx = (history_idx - k + NUM_TAPS) % NUM_TAPS;
            filtered_out += filter_taps[k] * history[idx];
        }
        
        history_idx = (history_idx + 1) % NUM_TAPS;
        total_energy += (filtered_out * filtered_out);
    }
    
    return total_energy / (STEREO_BUFFER_SIZE / 2);
}

int main() {
    set_sys_clock_khz(132000, true);
    stdio_init_all();
    sleep_ms(2000);

    printf("Starting FIR Filter in Normalized Debug Mode...\n");

    i2s_program_start_synched(pio0, &config, dma_i2s_in_handler, &i2s);

    while (true) {
        if (buffer_ready) {
            buffer_ready = false;
            const int32_t *buf = (const int32_t *)ready_buffer;
            
            // Get current energy in the 500 Hz band using the FIR filter output
            float energy = get_500hz_energy(buf) * 100.0f;
            
            // [NEW: Noise Gate]
            const float NOISE_GATE_THRESHOLD = 0.0001f;

            if (energy < NOISE_GATE_THRESHOLD) {
                energy = 0.0f; // Below threshold, force to zero!
            }

            // This is the threshold to detect 500 Hz
            const float DETECT_THRESHOLD = 0.00030f;

            if (energy > DETECT_THRESHOLD) {
                printf("\r[*** 500 Hz DETECTED ***] Energy: %10.6f  ", energy);
            } else {
                printf("\rListening... Energy: %10.6f             ", energy);
            }
            stdio_flush();
        }
    }

    return 0;
}