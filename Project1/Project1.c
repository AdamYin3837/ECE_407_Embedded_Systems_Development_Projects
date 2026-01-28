#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"

// Rotary encoder defines
#define ENCODER_A_PIN 14
#define ENCODER_B_PIN 15
#define ENCODER_SW_PIN 22
#define ENCODER_LED_R_PIN 18
#define ENCODER_LED_G_PIN 19
#define ENCODER_LED_B_PIN 20

// Global variables
volatile int encoder_count = 0;
volatile uint32_t last_interrupt_time = 0;
volatile bool encoder_changed = false; // New flag for printing outside ISR

#define DEBOUNCE_TIME_MS 15

// ============================================================================
// GPIO Initialization
// ============================================================================
void gpio_init_rotary_encoder(void) {
    gpio_init(ENCODER_A_PIN);
    gpio_init(ENCODER_B_PIN);
    
    gpio_set_dir(ENCODER_A_PIN, GPIO_IN);
    gpio_set_dir(ENCODER_B_PIN, GPIO_IN);
    
    gpio_pull_up(ENCODER_A_PIN);
    gpio_pull_up(ENCODER_B_PIN);
    
    printf("GPIO Initialized: A=%d, B=%d\n", ENCODER_A_PIN, ENCODER_B_PIN);
}

// ============================================================================
// RGB LED Initialization with PWM
// ============================================================================
void rgb_led_init(void) {
    // Set GPIO function to PWM
    gpio_set_function(ENCODER_LED_R_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ENCODER_LED_G_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ENCODER_LED_B_PIN, GPIO_FUNC_PWM);
    
    // Configure PWM slices
    uint slice_r = pwm_gpio_to_slice_num(ENCODER_LED_R_PIN);
    uint slice_g = pwm_gpio_to_slice_num(ENCODER_LED_G_PIN);
    uint slice_b = pwm_gpio_to_slice_num(ENCODER_LED_B_PIN);
    
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.0);
    pwm_config_set_wrap(&config, 255);
    
    pwm_init(slice_r, &config, true);
    pwm_init(slice_g, &config, true);
    pwm_init(slice_b, &config, true);
    
    printf("RGB LED Initialized on pins R=%d, G=%d, B=%d\n", ENCODER_LED_R_PIN, ENCODER_LED_G_PIN, ENCODER_LED_B_PIN);
}

// ============================================================================
// RGB LED Color Control
// ============================================================================
void set_rgb_color(uint8_t r, uint8_t g, uint8_t b) {
    // Invert values for Common Anode LED
    // (255 - value) makes 0 the brightest and 255 the darkest
    pwm_set_gpio_level(ENCODER_LED_R_PIN, 255 - r);
    pwm_set_gpio_level(ENCODER_LED_G_PIN, 255 - g);
    pwm_set_gpio_level(ENCODER_LED_B_PIN, 255 - b);
}

void update_led_color_from_position(int position) {
    // Normalize position to 0-359 for color wheel (HSV hue)
    int normalized_pos = ((position % 360) + 360) % 360;
    
    uint8_t r, g, b;
    
    // Simple color mapping: red -> yellow -> green -> cyan -> blue -> magenta -> red
    if (normalized_pos < 60) {
        // Red to Yellow
        r = 255;
        g = (normalized_pos * 255) / 60;
        b = 0;
    } else if (normalized_pos < 120) {
        // Yellow to Green
        r = (255 * (120 - normalized_pos)) / 60;
        g = 255;
        b = 0;
    } else if (normalized_pos < 180) {
        // Green to Cyan
        r = 0;
        g = 255;
        b = ((normalized_pos - 120) * 255) / 60;
    } else if (normalized_pos < 240) {
        // Cyan to Blue
        r = 0;
        g = (255 * (240 - normalized_pos)) / 60;
        b = 255;
    } else if (normalized_pos < 300) {
        // Blue to Magenta
        r = ((normalized_pos - 240) * 255) / 60;
        g = 0;
        b = 255;
    } else {
        // Magenta to Red
        r = 255;
        g = 0;
        b = (255 * (360 - normalized_pos)) / 60;
    }
    
    set_rgb_color(r, g, b);
    printf("Position: %d | RGB(%d, %d, %d)\n", position, r, g, b);
}

// ============================================================================
// Interrupt-Based Approach
// ============================================================================
void gpio_interrupt_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if (current_time - last_interrupt_time < DEBOUNCE_TIME_MS) {
        return;
    }
    last_interrupt_time = current_time;
    
    bool a_state = gpio_get(ENCODER_A_PIN);
    bool b_state = gpio_get(ENCODER_B_PIN);
    
    if (a_state != b_state) {
        encoder_count++;
    } else {
        encoder_count--;
    }
    
    encoder_changed = true;
}

void gpio_setup_interrupt_based(void) {
    gpio_init_rotary_encoder();
    
    gpio_set_irq_enabled_with_callback(ENCODER_A_PIN, 
                                      GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                                      true,
                                      &gpio_interrupt_handler);
    
    printf("Interrupt-based mode enabled\n");
}

// ============================================================================
// Polling-Based Approach (Fixed)
// ============================================================================
void polling_based_read_encoder(void) {
    static bool prev_a = true;
    
    bool curr_a = gpio_get(ENCODER_A_PIN);
    bool curr_b = gpio_get(ENCODER_B_PIN);
    
    if (curr_a != prev_a) {
        if (curr_a != curr_b) {
            encoder_count++; // FIX: Update the GLOBAL variable
        } else {
            encoder_count--; // FIX: Update the GLOBAL variable
        }
        encoder_changed = true; // FIX: Set flag so main loop knows to print
        prev_a = curr_a;
    }
}

// ============================================================================
// Main Function
// ============================================================================
int main(void) {
    stdio_init_all();
    sleep_ms(2000); 
    
    printf("\n=== Rotary Encoder Test ===\n");
    
    rgb_led_init();
    
    #define MODE_INTERRUPT 1
    #define MODE_POLLING   0
    #define SELECTED_MODE  MODE_INTERRUPT
    
    // FIX: Corrected Logic. Check for INTERRUPT mode first.
    if (SELECTED_MODE == MODE_INTERRUPT) {
        printf("Starting INTERRUPT-based mode...\n\n");
        gpio_setup_interrupt_based();
        
        while (true) {
            if (encoder_changed) {
                printf("Count: %d\n", encoder_count);
                update_led_color_from_position(encoder_count);
                encoder_changed = false;
            }
            tight_loop_contents(); 
        }
    } else {
        printf("Starting POLLING-based mode...\n\n");
        gpio_init_rotary_encoder();
        
        while (true) {
            polling_based_read_encoder();
            
            // FIX: Only update LED/Print if changed
            if (encoder_changed) {
                update_led_color_from_position(encoder_count);
                encoder_changed = false;
            }
            
            sleep_us(100); 
        }
    }
    
    return 0;
}
