#include "ws2812_led.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "ws2812.pio.h"

// ============================================================================
// Static State Variables
// ============================================================================

static PIO ws2812_pio = pio0;
static uint ws2812_sm = 0;
static uint ws2812_offset = 0;
static uint32_t led_buffer[WS2812_NUM_LEDS];

// ============================================================================
// Helper Functions
// ============================================================================

static inline int normalize_degrees(int degrees) {
  int normalized = degrees % 360;
  return (normalized < 0) ? (normalized + 360) : normalized;
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
}

static inline void ws2812_put_pixel(uint32_t pixel_grb) {
  pio_sm_put_blocking(ws2812_pio, ws2812_sm, pixel_grb << 8u);
}

// ============================================================================
// WS2812 Initialization and Control
// ============================================================================

void ws2812_init(void) {
  ws2812_offset = pio_add_program(ws2812_pio, &ws2812_program);
  ws2812_program_init(ws2812_pio, ws2812_sm, ws2812_offset, WS2812_PIN,
                      WS2812_FREQ, WS2812_IS_RGBW);
  ws2812_clear();
}

void ws2812_set_pixel(int index, uint8_t r, uint8_t g, uint8_t b) {
  if (index >= 0 && index < WS2812_NUM_LEDS) {
    led_buffer[index] = urgb_u32(r, g, b);
  }
}

void ws2812_clear(void) {
  for (int i = 0; i < WS2812_NUM_LEDS; ++i) {
    led_buffer[i] = 0;
  }
}

void ws2812_show(void) {
  for (int i = 0; i < WS2812_NUM_LEDS; ++i) {
    ws2812_put_pixel(led_buffer[i]);
  }
}

void ws2812_update_angle(int angle_degrees) {
  int normalized_deg = normalize_degrees(angle_degrees);
  int index = (normalized_deg * WS2812_NUM_LEDS) / 360;

  ws2812_clear();
  // Green at low brightness for legacy support
  ws2812_set_pixel(index, 0, 32, 0);
  ws2812_show();
}

void ws2812_fill_color(uint8_t r, uint8_t g, uint8_t b) {
  uint32_t color = urgb_u32(r, g, b);
  for (int i = 0; i < WS2812_NUM_LEDS; ++i) {
    led_buffer[i] = color;
  }
  ws2812_show();
}
