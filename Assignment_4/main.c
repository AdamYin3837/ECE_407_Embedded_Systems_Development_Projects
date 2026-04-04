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

#define NUM_TAPS 51

const float filter_taps[NUM_TAPS] = {
    -0.00027358323023720125f,    -9.267010455970655e-19f,    0.0003238978323318446f,    0.0007695719162695162f,    0.0014038891318421832f,
    0.0022862895336707187f,    0.0034662238697144875f,    0.00498095795489713f,    0.0068538277465600905f,    0.009093012370233806f,
    0.01169087319480083f,    0.014623886066987585f,    0.01785317176792521f,    0.021325607472085156f,    0.024975480304326177f,
    0.028726623816159078f,    0.03249496009811848f,    0.03619135499152997f,    0.039724682034230366f,    0.04300498281751531f,
    0.045946607647725284f,    0.048471220941304474f,    0.050510560620996826f,    0.05200884974593728f,    0.05292477136785802f,
    0.053232933685213785f,    0.05292477136785802f,    0.05200884974593727f,    0.05051056062099682f,    0.04847122094130447f,
    0.04594660764772528f,    0.043004982817515296f,    0.039724682034230366f,    0.03619135499152996f,    0.03249496009811847f,
    0.028726623816159074f,    0.02497548030432617f,    0.021325607472085145f,    0.017853171767925204f,    0.014623886066987574f,
    0.01169087319480083f,    0.009093012370233806f,    0.006853827746560088f,    0.004980957954897124f,    0.0034662238697144828f,
    0.002286289533670715f,    0.0014038891318421832f,    0.0007695719162695158f,    0.00032389783233184436f,    -9.267010455970647e-19f,
    -0.00027358323023720125f,
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
            
            float energy = get_500hz_energy(buf);
            
            // 門檻值大幅調降！你可以先看印出來的數字，再來微調這個 0.005f
            if (energy > 0.005f) {
                // 將 %10.0f 改為 %10.6f，因為現在數值很小，需要看小數點後面的變化
                printf("\r[*** 500 Hz DETECTED ***] Energy: %10.6f  ", energy);
            } else {
                printf("\rListening... Energy: %10.6f             ", energy);
            }
            stdio_flush();
        }
    }

    return 0;
}