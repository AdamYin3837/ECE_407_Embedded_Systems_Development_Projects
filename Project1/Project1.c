#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "encoder.h"
#include "rgb_led.h"
#include "ws2812_led.h"

// ============================================================================
// Main Function
// ============================================================================

int main(void) {
    stdio_init_all();
    sleep_ms(2000); 
    
    printf("\n=== PIO-based Rotary Encoder Test ===\n");
    
    // Initialize all hardware modules
    rgb_led_init();
    ws2812_init();
    
    printf("Starting Interrupt-based encoder...\n\n");
    encoder_init();
    encoder_button_init();

    // Initialize to -1 to force an update on the first loop iteration
    int last_angle = -1; 
    
    while (true) {
        // Poll encoder state machine FIFO for updates
        encoder_update();
        
        // 1. Handle Encoder Rotation
        if (encoder_changed || last_angle == -1) {
            int angle = normalize_degrees(encoder_count * ENCODER_DEG_PER_STEP);
            
            if (angle != last_angle) {
                printf("Angle: %d deg\n", angle);
                
                // Disable interrupts during WS2812 update to prevent timing glitches
                uint32_t irq_status = save_and_disable_interrupts();
                ws2812_update_angle(angle);
                restore_interrupts(irq_status);
                
                // Update the PWM RGB LED
                update_led_color_from_position(angle); 
                
                last_angle = angle;
            }
            encoder_changed = false;
        }

        // 2. Handle Button Press (Safe Context)
        if (button_pressed_flag) {
            // Reset flag
            button_pressed_flag = false;
            
            printf("Button press detected\n");

            // Calculate current color based on angle
            int angle = normalize_degrees(encoder_count * ENCODER_DEG_PER_STEP);
            uint8_t r, g, b;
            get_rgb_from_position(angle, &r, &g, &b);

            // Update WS2812 safely
            uint32_t irq_status = save_and_disable_interrupts();
            ws2812_fill_color(r, g, b);
            restore_interrupts(irq_status);
        }
        
        sleep_ms(1); 
    }
    
    return 0;
}
