#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// Game Logic Interface
// ============================================================================

/**
 * Initialize game state
 */
void game_init(void);

/**
 * Update game logic (should be called in the main loop)
 * Handles state transitions, timing, and input processing
 */
void game_update(void);

/**
 * Render the current game state to the LED ring
 */
void game_render(void);

#endif // GAME_H
