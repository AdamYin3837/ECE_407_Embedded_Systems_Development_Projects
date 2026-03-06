/**
  * @file main.c
  * @author umtariq, yanyong
  * @brief Reads an NTC thermistor and a potentiometer using the Raspberry Pi Pico ADC.
  * @date 2026-03-04
  * @license MIT License
  * @copyright Copyright (c) 2026
  */

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// Define our constants as floats to prevent integer truncation later
#define BETA 3950.0 
#define ADC_FULL_SCALE 4095.0

int main() {
    // 1. Initialize standard I/O for terminal printing
    stdio_init_all();

    // 2. Initialize the ADC hardware on the Pico
    adc_init();

    // 3. Set GPIO pins 26 and 27 to act as analogue inputs
    adc_gpio_init(26); // Connected to Potentiometer (ADC0)
    adc_gpio_init(27); // Connected to Thermistor (ADC1)

    while (true) {
        // --- READ POTENTIOMETER ---
        // Select ADC input 0 (which maps to GPIO 26)
        adc_select_input(0); 
        uint16_t pot_raw = adc_read();
        
        // Convert the raw 12-bit value (0-4095) to a voltage (0.0 - 3.3V)
        float pot_volts = pot_raw * (3.3 / ADC_FULL_SCALE);

        // --- READ THERMISTOR ---
        // Select ADC input 1 (which maps to GPIO 27)
        adc_select_input(1); 
        uint16_t th_r_divider = adc_read();
        
        float celsius = 0.0; // Default fallback

        // Safety check to prevent dividing by zero if the reading acts up
        if (th_r_divider > 0 && th_r_divider < 4095) {
            // The formula provided in your lab instructions
            celsius = 1.0 / (log(1.0 / (ADC_FULL_SCALE / (float)th_r_divider - 1.0)) / BETA + 1.0 / 298.15) - 273.15;
        }

        // --- OUTPUT TO TERMINAL ---
        printf("Set-point Voltage: %.2f V  |  Current Temp: %.2f C\n", pot_volts, celsius);

        // Wait exactly 1 second before reading again
        sleep_ms(1000);

    }

    return 0;
}