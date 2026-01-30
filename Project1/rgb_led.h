#ifndef RGB_LED_H
#define RGB_LED_H

#include <stdint.h>

// ============================================================================
// Configuration Defines
// ============================================================================

#define ENCODER_LED_R_PIN 18
#define ENCODER_LED_G_PIN 19
#define ENCODER_LED_B_PIN 20

// ============================================================================
// Function Declarations
// ============================================================================

/**
 * Initialize the RGB LED (PWM configuration)
 */
void rgb_led_init(void);

/**
 * Set RGB LED color with raw values (0-255)
 * For Common Anode LED: 255 = off, 0 = on (inverted)
 */
void set_rgb_color(uint8_t r, uint8_t g, uint8_t b);

/**
 * Update RGB LED based on position in degrees (0-360)
 * Uses a color wheel for smooth color transitions
 */
void update_led_color_from_position(int position);

/**
 * Get RGB values from a position in degrees (0-360)
 * Uses HSV color wheel mapping
 */
void get_rgb_from_position(int position, uint8_t *r, uint8_t *g, uint8_t *b);

#endif // RGB_LED_H
