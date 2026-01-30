#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "encoder.h"

// ============================================================================
// Global Variables
// ============================================================================

volatile int encoder_count = 0;
volatile bool encoder_changed = false;

volatile uint32_t last_button_interrupt_time = 0;
volatile bool button_pressed_flag = false;

static uint8_t last_ab_state = 0;

// Gray-code transition table (last<<2 | current) -> delta
// Valid sequence: 00 -> 01 -> 11 -> 10 -> 00 (clockwise)
static const int8_t encoder_transition_table[16] = {
    0,  1, -1,  0,
    -1,  0,  0,  1,
    1,  0,  0, -1,
    0,  -1, 1,  0
};

// ============================================================================
// Interrupt Handler (Shared for A, B, and SW)
// ============================================================================

static void encoder_gpio_callback(uint gpio, uint32_t events) {
    // 1. Handle Encoder Rotation
    if (gpio == ENCODER_A_PIN || gpio == ENCODER_B_PIN) {
        uint8_t a_state = gpio_get(ENCODER_A_PIN);
        uint8_t b_state = gpio_get(ENCODER_B_PIN);
        uint8_t current_ab = (b_state << 1) | a_state;

        if (current_ab != last_ab_state) {
            uint8_t index = (last_ab_state << 2) | current_ab;
            int8_t delta = encoder_transition_table[index];
            
            if (delta != 0) {
                encoder_count += delta;
                encoder_changed = true;
            }
            last_ab_state = current_ab;
        }
    }
    
    // 2. Handle Button Press
    if (gpio == ENCODER_SW_PIN) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());

        // Debounce Logic
        if (current_time - last_button_interrupt_time < BUTTON_DEBOUNCE_TIME_MS) {
            return;
        }
        last_button_interrupt_time = current_time;

        // Check if it's a Falling Edge (Press)
        if (events & GPIO_IRQ_EDGE_FALL) {
            button_pressed_flag = true;
        }
    }
}

// ============================================================================
// Initialization
// ============================================================================

void encoder_init(void) {
    // Initialize encoder pins
    gpio_init(ENCODER_A_PIN);
    gpio_init(ENCODER_B_PIN);
    gpio_set_dir(ENCODER_A_PIN, GPIO_IN);
    gpio_set_dir(ENCODER_B_PIN, GPIO_IN);
    gpio_pull_up(ENCODER_A_PIN);
    gpio_pull_up(ENCODER_B_PIN);
    
    // Read initial state
    uint8_t a = gpio_get(ENCODER_A_PIN);
    uint8_t b = gpio_get(ENCODER_B_PIN);
    last_ab_state = (b << 1) | a;

    printf("Encoder Interrupt Initialized: A=%d, B=%d\n", ENCODER_A_PIN, ENCODER_B_PIN);

    // Enable interrupts for A and B
    gpio_set_irq_enabled_with_callback(ENCODER_A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_gpio_callback);
    gpio_set_irq_enabled(ENCODER_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

void encoder_button_init(void) {
    gpio_init(ENCODER_SW_PIN);
    gpio_set_dir(ENCODER_SW_PIN, GPIO_IN);
    gpio_pull_down(ENCODER_SW_PIN);
    
    // Enable interrupt for SW (using same callback, already set if init called first)
    // If init not called, this sets it.
    // To be safe, we re-register callback if it wasn't set, but gpio_set_irq_enabled uses existing callback if set.
    // But we should use gpio_set_irq_enabled instead of with_callback if we trust init was called.
    // However, to correspond with proper initialization order independence (mostly), we can just set it.
    // But `gpio_set_irq_enabled_with_callback` updates the global callback.
    // Let's use `gpio_set_irq_enabled` if we assume `encoder_init` called first or similar.
    // Actually, Project1.c calls encoder_init then encoder_button_init.
    
    gpio_set_irq_enabled(ENCODER_SW_PIN, GPIO_IRQ_EDGE_FALL, true);
    
    printf("Button Initialized: SW=%d\n", ENCODER_SW_PIN);
}

// ============================================================================
// Update Loop
// ============================================================================

void encoder_update(void) {
    // No polling needed for interrupt-based encoder
    // Everything happens in `encoder_gpio_callback`
}
