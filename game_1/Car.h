#ifndef CAR_H
#define CAR_H

#include <stdint.h>
#include "Joystick.h"

// Screen constants
#define CAR_SCREEN_CENTRE_X   120
#define CAR_SCREEN_BOTTOM_Y   240
#define CAR_SPRITE_W           64
#define CAR_SPRITE_H           64
#define CAR_SCREEN_Y  (CAR_SCREEN_BOTTOM_Y - CAR_SPRITE_H - 8)  // 168

// Car physics constants
// Maximum lateral position, normalised to road half-width
// 1.0 = road edge, 2.0 = well into the grass
#define CAR_MAX_POS_X          2.0f
#define CAR_STEER_SPEED        1.2f   // fixed lateral units per second when steering
#define CAR_STEER_DEADZONE     0.5f   // magnitude must exceed this to register input

// Car state
typedef struct {
    float pos_x;             // Normalised lateral position: -1=left edge, 0=centre, +1=right edge
    float curvature_offset;  // Road curvature push - set by Track each frame. car_update adds this to pos_x
} Car_t;

// Initialise car at road centre
void car_init(Car_t *car);

/**
 * Update car lateral position from joystick input and curvature.
 *
 * @param car      Pointer to car state
 * @param input    UserInput from Joystick_GetInput() - uses magnitude and angle
 * @param dt_ms    Frame delta time in milliseconds
 */
void car_update(Car_t *car, UserInput input, uint32_t dt_ms);

// Draw the car sprite centred on screen
void car_draw(Car_t *car);

#endif // CAR_H