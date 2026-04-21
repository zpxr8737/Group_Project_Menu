#ifndef RACEENGINE_H
#define RACEENGINE_H

#include <stdint.h>
#include "Track.h"
#include "Car.h"

// Car sprite dimensions
#define CAR_SPRITE_W   64
#define CAR_SPRITE_H   64

// Sky sprite dimensions
#define SKY_SPRITE_WIDTH     300
#define SKY_SPRITE_HEIGHT    120
// Panning params
#define SKY_CROP_START_ROW    84
#define SKY_CROP_NUM_ROWS     36    // rows 84-119, sits just above horizon
#define SKY_PAN_RANGE         60    // 300 - 240 = 60px total pan range

// Draw rows 0-83 of the sky sprite statically
// These rows are centred horizontally in the 300px sprite, so take a 240px window from the middle: offset = (300-240)/2 = 30.
#define SKY_STATIC_ROWS     84    // rows 0-83
#define SKY_CENTRE_OFFSET   30    // centre a 240px window in 300px sprite

/**
 * @brief Engine state container
 */
typedef struct {
    uint8_t initialised;
    uint32_t last_tick_ms;    // HAL_GetTick() value at last Update call
    Car_t car;
    float sky_offset;    // accumulated curvature-driven sky pan
    int16_t last_pan;      // previous frame's pan offset, -1 forces first draw
} race_engine_t;

/**
 * @brief Initialise the engine and all subsystems.
 *        Must be called once before entering the game loop..
 *
 * @param engine  Pointer to the engine instance (must not be NULL).
 */
void race_engine_init(race_engine_t *engine);

/**
 * @brief Update all game logic for the current frame.
 *        Call before RaceEngine_Draw() each frame.
 *
 * @param engine  Pointer to the engine instance.
 */
void race_engine_update(race_engine_t *engine, uint8_t btn2_pressed);

/**
 * @brief Render the current frame to the LCD and flush to display.
 *
 * @param engine  Pointer to the engine instance.
 */
void race_engine_draw(race_engine_t *engine);

#endif // RACEENGINE_H
