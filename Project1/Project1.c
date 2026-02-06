#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Modules
#include "encoder.h"
#include "game.h"
#include "meter.h"
#include "wheel.h"
#include "rgb_led.h"
#include "ws2812_led.h"

// ============================================================================
// Configuration
// ============================================================================

// Menu Layout positions (Clock face)
#define OPTION_GAME_POS 3  // Green LED position
#define OPTION_METER_POS 9 // Red LED position
#define OPTION_WHEEL_POS 6 // Yellow LED position

// ============================================================================
// State Management
// ============================================================================

typedef enum { PROGRAM_MENU, PROGRAM_GAME, PROGRAM_METER, PROGRAM_WHEEL } ProgramMode_t;

static ProgramMode_t current_mode = PROGRAM_MENU;

// ============================================================================
// Menu Logic
// ============================================================================

static int menu_cursor = 0;

void menu_update(void) {
  // 1. Move Cursor with Encoder
  int raw_pos = encoder_count / ENCODER_STEPS_PER_DETENT;

  // Normalize to 0..11
  menu_cursor = raw_pos % WS2812_NUM_LEDS;
  if (menu_cursor < 0)
    menu_cursor += WS2812_NUM_LEDS;

  // 2. Check Selection (Click)
  if (button_pressed_flag) {
    button_pressed_flag = false; // Consume click

    if (menu_cursor == OPTION_GAME_POS) {
      current_mode = PROGRAM_GAME;
      game_init();

      // Visual Confirmation (Flash Green)
      ws2812_fill_color(0, 32, 0);
      ws2812_show();
      sleep_ms(300);
    } else if (menu_cursor == OPTION_METER_POS) {
      current_mode = PROGRAM_METER;
      meter_init();

      // Visual Confirmation (Flash Red)
      ws2812_fill_color(32, 0, 0);
      ws2812_show();
      sleep_ms(300);
    } else if (menu_cursor == OPTION_WHEEL_POS) {
      current_mode = PROGRAM_WHEEL;
      wheel_init();

      // Visual Confirmation (Flash Yellow)
      ws2812_fill_color(32, 32, 0);
      ws2812_show();
      sleep_ms(300);
    }
  }
}

void menu_render(void) {
  ws2812_clear();

  // Draw "Game" Option (Green)
  ws2812_set_pixel(OPTION_GAME_POS, 0, 32, 0);

  // Draw "Physics" Option (Red)
  ws2812_set_pixel(OPTION_METER_POS, 32, 0, 0);

  // Draw "Wheel" Option (Yellow)
  ws2812_set_pixel(OPTION_WHEEL_POS, 32, 32, 0);

  // Draw Blue Pointer (Cursor)
  // If cursor is ON an option, make it White to indicate "Selected"
  if (menu_cursor == OPTION_GAME_POS || menu_cursor == OPTION_METER_POS || menu_cursor == OPTION_WHEEL_POS) {
    ws2812_set_pixel(menu_cursor, 32, 32, 32);
  } else {
    ws2812_set_pixel(menu_cursor, 0, 0, 32);
  }

  ws2812_show();
}

// ============================================================================
// Main Function
// ============================================================================

int main(void) {
  stdio_init_all();
  sleep_ms(2000);
  printf("\n=== Pico Selection Menu ===\n");

  // Init Hardware
  rgb_led_init();
  ws2812_init();
  encoder_init();
  encoder_button_init();

  // Start in Menu Mode
  current_mode = PROGRAM_MENU;
  printf("Mode: MENU\n");

  while (true) {
    encoder_update(); // Driver update

    switch (current_mode) {
    case PROGRAM_MENU:
      menu_update();
      menu_render();
      break;

    case PROGRAM_GAME:
      game_update();
      game_render();
      break;

    case PROGRAM_METER:
      meter_update();
      meter_render();
      break;

    case PROGRAM_WHEEL:
      wheel_update();
      wheel_render();
      break;
    }

    sleep_ms(10);
  }

  return 0;
}
