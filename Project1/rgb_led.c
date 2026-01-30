#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "rgb_led.h"

// ============================================================================
// RGB LED Control Functions
// ============================================================================

void rgb_led_init(void) {
    // Configure GPIO for PWM
    gpio_set_function(ENCODER_LED_R_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ENCODER_LED_G_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ENCODER_LED_B_PIN, GPIO_FUNC_PWM);
    
    // Get PWM slices
    uint slice_r = pwm_gpio_to_slice_num(ENCODER_LED_R_PIN);
    uint slice_g = pwm_gpio_to_slice_num(ENCODER_LED_G_PIN);
    uint slice_b = pwm_gpio_to_slice_num(ENCODER_LED_B_PIN);
    
    // Configure PWM
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.0);
    pwm_config_set_wrap(&config, 255);
    
    pwm_init(slice_r, &config, true);
    pwm_init(slice_g, &config, true);
    pwm_init(slice_b, &config, true);
    
    printf("RGB LED Initialized on pins R=%d, G=%d, B=%d\n", ENCODER_LED_R_PIN, ENCODER_LED_G_PIN, ENCODER_LED_B_PIN);
}

void set_rgb_color(uint8_t r, uint8_t g, uint8_t b) {
    // Invert values for Common Anode LED (255 is off, 0 is on)
    pwm_set_gpio_level(ENCODER_LED_R_PIN, 255 - r);
    pwm_set_gpio_level(ENCODER_LED_G_PIN, 255 - g);
    pwm_set_gpio_level(ENCODER_LED_B_PIN, 255 - b);
}

void get_rgb_from_position(int position, uint8_t *r, uint8_t *g, uint8_t *b) {
    int normalized_pos = ((position % 360) + 360) % 360;

    // Color wheel mapping
    if (normalized_pos < 60) {
        *r = 255; *g = (normalized_pos * 255) / 60; *b = 0;
    } else if (normalized_pos < 120) {
        *r = (255 * (120 - normalized_pos)) / 60; *g = 255; *b = 0;
    } else if (normalized_pos < 180) {
        *r = 0; *g = 255; *b = ((normalized_pos - 120) * 255) / 60;
    } else if (normalized_pos < 240) {
        *r = 0; *g = (255 * (240 - normalized_pos)) / 60; *b = 255;
    } else if (normalized_pos < 300) {
        *r = ((normalized_pos - 240) * 255) / 60; *g = 0; *b = 255;
    } else {
        *r = 255; *g = 0; *b = (255 * (360 - normalized_pos)) / 60;
    }
}

void update_led_color_from_position(int position) {
    uint8_t r, g, b;
    get_rgb_from_position(position, &r, &g, &b);
    set_rgb_color(r, g, b);
}
