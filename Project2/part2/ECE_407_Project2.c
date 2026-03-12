/*
  Raspberry Pi Pico SDK PWM Example

  Use the logic analyzer to view the signals: https://docs.wokwi.com/guides/logic-analyzer
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#define POTENTIOMETER_PIN 26

int main() {
  stdio_init_all();
  adc_init();

  // Initialize PWM pins for the H-bridge / LEDs
  gpio_set_function(14, GPIO_FUNC_PWM);
  gpio_set_function(15, GPIO_FUNC_PWM);

  // Initialize ADC for the Potentiometer
  adc_gpio_init(POTENTIOMETER_PIN);
  adc_select_input(0);

  // Get the slice for GPIO 14 (Slice 0)
  uint slice_num = pwm_gpio_to_slice_num(14);

  // Set a more standard clock divider for motor PWM
  pwm_set_clkdiv(slice_num, 3.0f); // PWM clock should now be running at 1MHz
  pwm_set_wrap(slice_num, 4095);  // Set period of 1024 cycles (0 to 1023 inclusive)
  pwm_set_enabled(slice_num, true);

  while (1) {
    uint16_t result = adc_read();
    
    // Define our center point and a deadzone to prevent jittering
    const int center = 2048;
    const int deadzone = 100; 
    
    int duty_A = 0; // Forward PWM (Red LED)
    int duty_B = 0; // Reverse PWM (Blue LED)

    if (result > (center + deadzone)) {
      // Forward direction: Map upper half of pot to full PWM range
      duty_A = (result - (center + deadzone)) * 4095 / (4095 - (center + deadzone));
      duty_B = 0; // Ensure reverse is off
    } 
    else if (result < (center - deadzone)) {
      // Reverse direction: Map lower half of pot to full PWM range
      duty_B = ((center - deadzone) - result) * 4095 / (center - deadzone);
      duty_A = 0; // Ensure forward is off
    }

    // Safety clamps just in case the math exceeds bounds
    if (duty_A > 4095) duty_A = 4095;
    if (duty_B > 4095) duty_B = 4095;

    // Apply the duty cycles to the PWM channels
    pwm_set_chan_level(slice_num, PWM_CHAN_A, duty_A);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, duty_B);
    
    sleep_ms(50); // Slightly faster update rate feels more responsive
  }
}