#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// Configuration Defines
// ============================================================================

#define ENCODER_A_PIN 14
#define ENCODER_B_PIN 15
#define ENCODER_SW_PIN 21

#define ENCODER_DEG_PER_STEP 5
#define BUTTON_DEBOUNCE_TIME_MS 50

// ============================================================================
// Global Variables (accessible from other modules)
// ============================================================================

extern volatile int encoder_count;
extern volatile bool encoder_changed;
extern volatile bool button_pressed_flag;

// ============================================================================
// Function Declarations
// ============================================================================

/**
 * Initialize encoder using GPIO interrupts
 */
void encoder_init(void);

/**
 * Poll encoder state machine FIFO and update encoder_count
 * Call this in the main loop or from a timer
 */
void encoder_update(void);

/**
 * Initialize button for encoder switch (remains GPIO-based)
 */
void encoder_button_init(void);

/**
 * Normalize degrees to 0-359 range
 */
static inline int normalize_degrees(int degrees) {
    int normalized = degrees % 360;
    return (normalized < 0) ? (normalized + 360) : normalized;
}

#endif // ENCODER_H
