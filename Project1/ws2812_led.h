#ifndef WS2812_LED_H
#define WS2812_LED_H

#include <stdint.h>

// ============================================================================
// Configuration Defines
// ============================================================================

#define WS2812_PIN 27
#define WS2812_NUM_LEDS 12
#define WS2812_FREQ 800000
#define WS2812_IS_RGBW false

// ============================================================================
// Function Declarations
// ============================================================================

/**
 * Initialize the WS2812 LED ring with PIO
 */
void ws2812_init(void);

/**
 * Convert RGB values to GRB format (uint32_t)
 * WS2812 expects GRB order
 */
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);

/**
 * Put a single pixel color to the WS2812 buffer
 */
static inline void ws2812_put_pixel(uint32_t pixel_grb);


/**
 * Set a specific pixel in the buffer (does not update hardware immediately)
 * Call ws2812_show() to update the display
 * 
 * @param index Pixel index (0 to WS2812_NUM_LEDS-1)
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 */
void ws2812_set_pixel(int index, uint8_t r, uint8_t g, uint8_t b);

/**
 * Clear the internal LED buffer (set all to 0)
 * Call ws2812_show() to update the display
 */
void ws2812_clear(void);

/**
 * Push the current buffer to the WS2812 LEDs
 */
void ws2812_show(void);

/**
 * Update the WS2812 LED ring to show a single pixel at the given angle
 * Angle is in degrees (0-359). Clears other pixels.
 * Updates display immediately (legacy support)
 */
void ws2812_update_angle(int angle_degrees);

/**
 * Fill all WS2812 LEDs with a single color
 * Updates display immediately (legacy support)
 */
void ws2812_fill_color(uint8_t r, uint8_t g, uint8_t b);

#endif // WS2812_LED_H
