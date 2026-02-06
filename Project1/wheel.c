#include "wheel.h"
#include "encoder.h"
#include "rgb_led.h"
#include "ws2812_led.h"
#include "pico/stdlib.h"
#include <math.h>
#include <stdlib.h>

// ============================================================================
// Constants
// ============================================================================

// Physics Tuning
#define MASS 60.0f           
#define FRICTION 0.995f       
#define VELOCITY_SCALE 1.0f 

// RGB LED Tuning
#define SENSITIVITY 5      // How fast color changes with speed
#define NEUTRAL_RED 60      // Base Red for Yellow (Yellow = Red + Green)
#define NEUTRAL_GREEN 45    // Base Green for Yellow (Adjust if too lime/orange)

// ============================================================================
// State Variables
// ============================================================================

static int last_encoder_count = 0;
static float virtual_velocity = 0.0f;
static float virtual_position = 0.0f;

// ============================================================================
// Helper Functions
// ============================================================================

static float normalize_position(float pos) {
    while (pos >= WS2812_NUM_LEDS) pos -= WS2812_NUM_LEDS;
    while (pos < 0) pos += WS2812_NUM_LEDS;
    return pos;
}

static int clamp(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

// ============================================================================
// Main Logic
// ============================================================================

void wheel_init(void) {
    last_encoder_count = encoder_count;
    virtual_velocity = 0.0f;
    virtual_position = 0.0f;
    
    // Set initial neutral yellow
    set_rgb_color(NEUTRAL_RED, NEUTRAL_GREEN, 0);
}

void wheel_update(void) {
    // 1. Calculate Input Force
    int current_count = encoder_count;
    int delta = current_count - last_encoder_count;
    last_encoder_count = current_count;

    // 2. Calculate RGB Color
    // Start at Neutral Yellow
    int r = NEUTRAL_RED;
    int g = NEUTRAL_GREEN;
    int b = 0;

    int change = abs(delta) * SENSITIVITY;

    if (delta > 0) {
        // Clockwise -> Fade to GREEN
        // Reduce Red, Increase Green
        r -= change;
        g += change;
    } 
    else if (delta < 0) {
        // Counter-Clockwise -> Fade to RED
        // Increase Red, Reduce Green
        r += change;
        g -= change;
    }

    // Clamp values to valid LED range (0-255)
    set_rgb_color(clamp(r, 0, 255), clamp(g, 0, 255), b);


    // 3. Apply Physics
    virtual_velocity += (float)delta / MASS;
    virtual_velocity *= FRICTION;

    if (fabs(virtual_velocity) < 0.001f) virtual_velocity = 0;

    // 4. Update Position
    virtual_position += virtual_velocity;
    virtual_position = normalize_position(virtual_position);
}

void wheel_render(void) {
    ws2812_clear();

    // Anti-Aliased "Inertia Dot"
    float pos = virtual_position;
    int idx1 = (int)pos;
    int idx2 = (idx1 + 1) % WS2812_NUM_LEDS;
    float fraction = pos - idx1;
    
    int b1 = (int)((1.0f - fraction) * 32);
    int b2 = (int)(fraction * 32);

    ws2812_set_pixel(idx1, b1, b1, b1); 
    ws2812_set_pixel(idx2, b2, b2, b2); 

    ws2812_show();
}
