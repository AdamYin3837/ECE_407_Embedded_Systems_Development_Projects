#include "game.h"
#include "encoder.h"
#include "pico/stdlib.h"
#include "rgb_led.h"
#include "ws2812_led.h"
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// Constants & Configuration
// ============================================================================

// Original Player Colors (kept for reference, but we override them dynamically
// now)
#define PLAYER_COLOR_R 0
#define PLAYER_COLOR_G 32
#define PLAYER_COLOR_B 0

#define TARGET_COLOR_R 32
#define TARGET_COLOR_G 0
#define TARGET_COLOR_B 0

#define TIMER_COLOR_R 16
#define TIMER_COLOR_G 16
#define TIMER_COLOR_B 0

#define WIN_FLASH_COLOR_R 0
#define WIN_FLASH_COLOR_G 32
#define WIN_FLASH_COLOR_B 0

#define BASE_TIME_MS 10000
#define MIN_TIME_MS 1000
#define TIME_DECREMENT_MS 500

// ============================================================================
// State Definitions
// ============================================================================

typedef enum { STATE_IDLE, STATE_PLAYING, STATE_GAME_OVER } GameState_t;

static GameState_t current_state = STATE_IDLE;

// Game State Variables
static int player_pos = 0; // 0-11
static int target_pos = 0; // 0-11
static uint32_t time_left_ms = 0;
static uint32_t max_time_ms = BASE_TIME_MS;
static uint32_t last_update_time = 0;
static int level = 0;

// Speedometer Variables (NEW)
static int last_encoder_count = 0;
static int current_velocity = 0;

// Idle Animation State
static int idle_anim_pos = 0;
static uint32_t last_idle_time = 0;

// Game Over Animation State
static int game_over_flash_count = 0;
static uint32_t last_flash_time = 0;
static bool flash_state = false;

// ============================================================================
// Helper Functions
// ============================================================================

static int normalize_pos(int pos) {
  int res = pos % WS2812_NUM_LEDS;
  return (res < 0) ? (res + WS2812_NUM_LEDS) : res;
}

static void spawn_target(void) {
  // Random position different from player
  do {
    target_pos = rand() % WS2812_NUM_LEDS;
  } while (target_pos == player_pos);
}

static void reset_game(void) {
  level = 0;
  max_time_ms = BASE_TIME_MS;
  time_left_ms = max_time_ms;
  spawn_target();
  last_update_time = to_ms_since_boot(get_absolute_time());

  // Reset encoder tracking
  last_encoder_count = encoder_count;
  current_velocity = 0;
}

static void next_level(void) {
  level++;
  max_time_ms -= TIME_DECREMENT_MS;
  if (max_time_ms < MIN_TIME_MS)
    max_time_ms = MIN_TIME_MS;
  time_left_ms = max_time_ms;
  spawn_target();

  // Quick green flash or feedback could be nice here
  ws2812_fill_color(0, 50, 0);
  sleep_ms(100);
}

// ============================================================================
// Main Game Logic
// ============================================================================

void game_init(void) {
  srand(to_ms_since_boot(get_absolute_time()));
  current_state = STATE_IDLE;
  last_encoder_count = 0; // Initialize
}

void game_update(void) {
  uint32_t now = to_ms_since_boot(get_absolute_time());

  // --- NEW: Calculate Velocity ---
  // Determine how many steps changed since the last update loop
  int raw_diff = encoder_count - last_encoder_count;
  current_velocity =
      abs(raw_diff); // Absolute speed (direction doesn't matter for color)
  last_encoder_count = encoder_count; // Reset for next frame

  // Update player position from encoder
  // One click (detent) = One LED. Usually 4 steps per detent.
  player_pos = normalize_pos(encoder_count / ENCODER_STEPS_PER_DETENT);

  // Handle Button Input
  bool btn_pressed = false;
  if (button_pressed_flag) {
    button_pressed_flag = false;
    btn_pressed = true;
  }

  switch (current_state) {
  case STATE_IDLE:
    if (btn_pressed) {
      current_state = STATE_PLAYING;
      reset_game();
    }
    break;

  case STATE_PLAYING:
    // Timer Logic
    if (now - last_update_time > 10) { // 10ms tick
      uint32_t delta = now - last_update_time;
      if (time_left_ms > delta) {
        time_left_ms -= delta;
      } else {
        time_left_ms = 0;
        current_state = STATE_GAME_OVER;
        game_over_flash_count = 0;
        flash_state = false;
      }
      last_update_time = now;
    }

    // Win Condition
    if (btn_pressed) {
      if (player_pos == target_pos) {
        next_level();
      } else {
        // Missed target -> Lose logic (optional)
        // current_state = STATE_GAME_OVER;
        // game_over_flash_count = 0;
        // flash_state = false;
      }
    }
    break;

  case STATE_GAME_OVER:
    if (game_over_flash_count >= 6) { // 3 flashes (on/off * 3)
      if (btn_pressed) {
        current_state = STATE_PLAYING;
        reset_game();
      }
    }
    break;
  }
}

void game_render(void) {
  ws2812_clear();

  switch (current_state) {
  case STATE_IDLE: {
    // Simple blue spinning animation
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_idle_time > 100) {
      idle_anim_pos = (idle_anim_pos + 1) % WS2812_NUM_LEDS;
      last_idle_time = now;
    }
    ws2812_set_pixel(idle_anim_pos, 0, 0, 32);
    break;
  }

  case STATE_PLAYING: {
    // 1. Render Target
    ws2812_set_pixel(target_pos, TARGET_COLOR_R, TARGET_COLOR_G,
                     TARGET_COLOR_B);

    // 2. Render Player (Dynamic Speed Color)
    // Map speed to color: Slow = Blue, Fast = Green
    const int MAX_BRIGHTNESS = 40;
    const int SPEED_THRESHOLD = 3; // Max speed (steps per 10ms)

    int safe_velocity = current_velocity;
    if (safe_velocity > SPEED_THRESHOLD) {
      safe_velocity = SPEED_THRESHOLD;
    }

    // Calculate mix ratio
    // Higher speed = More Green, Less Blue
    int green_val = (safe_velocity * MAX_BRIGHTNESS) / SPEED_THRESHOLD;
    int blue_val = MAX_BRIGHTNESS - green_val;

    ws2812_set_pixel(player_pos, 0, green_val, blue_val);

    // 3. Render Timer (Yellow dots around target)
    int leds_available = WS2812_NUM_LEDS - 1; // Exclude target
    int max_pairs = leds_available / 2;
    int pairs_lit = (time_left_ms * max_pairs) / max_time_ms;

    // Draw pairs expanding outwards from target
    for (int i = 1; i <= pairs_lit; ++i) {
      // Right side
      int right_idx = normalize_pos(target_pos + i);
      if (right_idx != player_pos) {
        ws2812_set_pixel(right_idx, TIMER_COLOR_R, TIMER_COLOR_G,
                         TIMER_COLOR_B);
      }

      // Left side
      int left_idx = normalize_pos(target_pos - i);
      if (left_idx != player_pos) {
        ws2812_set_pixel(left_idx, TIMER_COLOR_R, TIMER_COLOR_G, TIMER_COLOR_B);
      }
    }
    break;
  }

  case STATE_GAME_OVER: {
    // Flash Red
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_flash_time > 200 && game_over_flash_count < 6) {
      flash_state = !flash_state;
      game_over_flash_count++;
      last_flash_time = now;
    }

    if (flash_state || game_over_flash_count >= 6) {
      if (game_over_flash_count < 6) {
        for (int i = 0; i < WS2812_NUM_LEDS; ++i) {
          ws2812_set_pixel(i, 32, 0, 0);
        }
      } else {
        // Show just 4 red LEDs in cross pattern
        for (int i = 0; i < WS2812_NUM_LEDS; i += 3) {
          ws2812_set_pixel(i, 32, 0, 0);
        }
      }
    }
    break;
  }
  }

  ws2812_show();
}
