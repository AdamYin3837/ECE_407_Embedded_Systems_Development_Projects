#include "pico/stdlib.h"
#include "ws2812_led.h"
#include "encoder.h"
#include <stdlib.h>
#include <math.h>

// ============================================================================
// Configuration
// ============================================================================

// Colors
#define VEL_R 0
#define VEL_G 32
#define VEL_B 32

#define ACC_R 64
#define ACC_G 16
#define ACC_B 0

// Scales (Adjust sensitivity here)
#define MAX_VELOCITY_SCALE 8 
#define MAX_ACCEL_SCALE 4

// Smoothing (Lower = Slower fade out)
#define DECAY_RATE_VEL 0.05f  
#define DECAY_RATE_ACC 0.1f 

// ============================================================================
// State Variables
// ============================================================================

static int last_encoder_count = 0;
static int prev_velocity = 0;

// We use float for the display values to allow for smooth fractional fading
static float filtered_velocity = 0.0f;
static float filtered_accel = 0.0f;

// ============================================================================
// Main Logic
// ============================================================================

void meter_init(void) {
    last_encoder_count = encoder_count;
    prev_velocity = 0;
    filtered_velocity = 0;
    filtered_accel = 0;
}

void meter_update(void) {
    // 1. Calculate Raw Physics
    int current_count = encoder_count;
    int raw_diff = current_count - last_encoder_count;
    int current_velocity = abs(raw_diff); 
    int current_accel = abs(current_velocity - prev_velocity);

    // 2. Smooth Velocity (Instant Rise, Slow Decay)
    if (current_velocity > filtered_velocity) {
        filtered_velocity = (float)current_velocity; // Jump up immediately
    } else {
        filtered_velocity -= DECAY_RATE_VEL; // Slide down slowly
        if (filtered_velocity < 0) filtered_velocity = 0;
    }

    // 3. Smooth Acceleration (Instant Rise, Very Slow Decay)
    // Accel is naturally spiky, so we hold the peak longer to make it visible
    if (current_accel > filtered_accel) {
        filtered_accel = (float)current_accel;
    } else {
        filtered_accel -= DECAY_RATE_ACC;
        if (filtered_accel < 0) filtered_accel = 0;
    }

    // 4. Update History
    last_encoder_count = current_count;
    prev_velocity = current_velocity;
}

void meter_render(void) {
    ws2812_clear();

    int half_leds = WS2812_NUM_LEDS / 2;

    // --- RENDER VELOCITY (Right Side) ---
    // Cast the smoothed float back to int for LED counting
    int vel_leds_lit = (int)((filtered_velocity * half_leds) / MAX_VELOCITY_SCALE);
    if (vel_leds_lit > half_leds) vel_leds_lit = half_leds;

    for (int i = 0; i < vel_leds_lit; i++) {
        ws2812_set_pixel(i, VEL_R, VEL_G, VEL_B);
    }

    // --- RENDER ACCELERATION (Left Side) ---
    int acc_leds_lit = (int)((filtered_accel * half_leds) / MAX_ACCEL_SCALE);
    if (acc_leds_lit > half_leds) acc_leds_lit = half_leds;

    for (int i = 0; i < acc_leds_lit; i++) {
        int idx = WS2812_NUM_LEDS - 1 - i;
        ws2812_set_pixel(idx, ACC_R, ACC_G, ACC_B);
    }

    ws2812_show();
}
