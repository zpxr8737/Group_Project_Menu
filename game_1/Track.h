#ifndef TRACK_H
#define TRACK_H

#include <stdint.h>

// Screen geometry
#define SCREEN_WIDTH   240
#define SCREEN_HEIGHT  240
#define HORIZON_Y      0.5*SCREEN_HEIGHT    // Row where the sky meets the road (vanishing point row)
#define CENTRE_X       0.5*SCREEN_WIDTH    // Column of the vanishing point (centred on screen)

// Road geometry - all values are measured at the BOTTOM of the screen
#define ROAD_WIDTH     0.75*SCREEN_WIDTH

// Segment system
#define MAX_SEGS        512     // upper bound on track length
#define SEG_LEN         3       // world units per segment
#define DRAW_DIST       120     // segments drawn to the horizon

// Curvature parameters
#define CURVE_SCALE     4.0f    // pixel shift per unit of accumulated curvature offset
#define CURVE_CAR_PUSH  0.058f  // how strongly curvature drifts the car (normalised pos_x units per world-speed unit per second)

// Kerb sine parameters
#define KERB_SINE_FREQ       35.0f
#define KERB_SINE_POW         2.0f
#define KERB_SCROLL_SPEED     0.7f   // how fast kerb scrolls relative to camera_z

// Grass sine parameters 
#define GRASS_SINE_FREQ      20.0f
#define GRASS_SINE_POW        3.0f
#define GRASS_SCROLL_SPEED    0.1f   // scrolls slower than kerb

// Dash sine paramters
#define DASH_THRESHOLD         0.1f   // > 0.0f = shorter dashes, < 0.0f = longer dashes

// Speed parameters (world-units per second / per second²)
#define MAX_SPEED    50.0f   // top speed
#define ACCEL        25.0f   // acceleration rate (0 → max in ~2 s)
#define DECEL        35.0f   // coast-down rate  (max → 0 in ~5 s)

// Sprite parameters
#define SPRITE_MIN_SCALE     1
#define SPRITE_MAX_SCALE     3
#define SPRITE_DRAW_DIST     120     // reduced draw distance for sprites — half of DRAW_DIST
#define FRUIT_SPRITE_W       16
#define FRUIT_SPRITE_H       16
#define FRUIT_LANE_OFFSET    0.4f   // normalised lateral offset from centre (0=centre, 1=road edge)
#define MAX_FRUIT_COUNT      32     // upper bound: one per AddStraight/AddCurve call

// Lap parameters
#define NUM_LAPS    3

// Colour palette indices - These map to palette_custom[] in LCD.c
#define COLOUR_IDX_BLACK        1   
#define COLOUR_IDX_SKY          12  // default sky colour
#define COLOUR_IDX_GRASS_A      7   // alternating grass stripe colour A
#define COLOUR_IDX_GRASS_B      3   // alternating grass stripe colour B
#define COLOUR_IDX_ROAD         11  // road colour
#define COLOUR_IDX_KERB_RED     5   // kerb red
#define COLOUR_IDX_KERB_WHITE   15   // Also used for the centre-line dash

/**
 * @brief Initialise track state.
 */
void track_init(void);

/**
 * @brief // Called in race_engine_update to update track state. Update camera position, fruit collision state and speed.
 * @param dt_ms         Frame delta time in milliseconds
 * @param btn2_pressed  1 if BTN2 was pressed this frame (toggles throttle), 0 otherwise.
 * @param player_x      player lateral position
 */
void track_update(uint32_t dt_ms, uint8_t btn2_pressed, float player_x);

/**
 * @brief Render the full track and sprites into the LCD frame buffer.
 */
void track_draw(float player_x);


// Return fruits collected so far
int32_t track_get_fruits_collected(void);

// Return total fruits after track is built
int32_t track_get_total_fruits(void);

// Returns the current camera speed in world-units/second
float track_get_speed(void);

// Return camera/player world Z position
float track_get_camera_z(void);

// Returns the curvature of the segment the player is currently on
// Used by race_engine_update to apply drift to the car each frame
float track_get_curvature(void);

// Builds the track - Called from track_init
void  track_build(void);

// Return current lap
uint8_t  track_get_current_lap(void);

// Return lap time - called for each frame to update lap timer
uint32_t track_get_lap_time_ms(void);

// Return flag - used to check if all laps are complete
uint8_t  track_is_race_complete(void);

// Return fastest lap after all laps are complete
uint32_t track_get_best_lap_ms(void);

#endif // TRACK_H
