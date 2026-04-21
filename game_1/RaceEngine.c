#include "RaceEngine.h"
#include "LCD.h"
#include "Track.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdio.h>
#include "Car.h"
#include "sprites.h"
#include <string.h>
#include <InputHandler.h>

// cfg0 is defined in main.c
extern ST7789V2_cfg_t cfg0;
extern Joystick_cfg_t joystick_cfg;
extern Joystick_t joystick_data;

// Intro screen
static void display_intro(void) {
    // How to play
    LCD_Fill_Buffer(12);   // colour index 12 = background

    char strings[10][25] = { 
        "HOW TO PLAY:", 
        "You are getting late", 
        "for your 9am lecture", 
        "and have no time", 
        "to eat!",
        "You only have time",
        "for 3 laps.",
        "Grab as many fruits",
        "as you can!",
        "BTN3 TO CONTINUE",
    };

    // Screen y positions for each line - derived from trial and error
    uint32_t y_pos[10] = {10, 47, 64, 81, 98, 125, 142, 168, 186, 220};

    // Draw each line
    for (size_t i = 0; i < sizeof(strings) / sizeof(strings[0]); i++) {
        uint16_t l_x = (SCREEN_WIDTH - (uint16_t)(strlen(strings[i]) * 6 * 2)) / 2;
        LCD_printString(strings[i], l_x, y_pos[i], 10, 2);
    }

    LCD_Refresh(&cfg0);
    
    while (1) {
        // Read input
        Input_Read();
        // Check if button was pressed to continue to game
        if (current_input.btn3_pressed) {
            break;  // Exit intro
        }    
    }    
}

void race_engine_init(race_engine_t *engine) {
    LCD_Set_Palette(PALETTE_CUSTOM);

    // Splashscreen
    LCD_Draw_Sprite(0, 0, 240, 240, (uint8_t*)LFS_SPLASHSCREEN);
    LCD_Refresh(&cfg0);
    HAL_Delay(5000);  

    // Game intro
    display_intro();

    // Initialisation
    track_init();
    car_init(&engine->car);
    engine->last_tick_ms = HAL_GetTick();
    engine->sky_offset = (float)(SKY_PAN_RANGE / 2);  // start centred
    engine->last_pan = -1;       // keep track of last offset   

    // Draw full centred bcakground
    LCD_Draw_Sprite(0, 0, 120, 300, (uint8_t*)SKY + SKY_CENTRE_OFFSET);
    LCD_Refresh(&cfg0);
}

void race_engine_update(race_engine_t *engine, uint8_t btn2_pressed) {
    uint32_t now = HAL_GetTick();
    uint32_t dt_ms = now - engine->last_tick_ms;
    engine->last_tick_ms = now;

    track_update(dt_ms, btn2_pressed, engine->car.pos_x);

    // Curvature drift
    // Pushes the car in the direction of the current road curvature, used in car_update
    float dt = (float)dt_ms * 0.001f;
    engine->car.curvature_offset = track_get_curvature() * track_get_speed() * dt * CURVE_CAR_PUSH;

    // Accumulate curvature weighted by speed and constant scaler
    // Negative sign: right-hand curve (sC > 0) pans sky left (offset decreases)
    engine->sky_offset += track_get_curvature() * track_get_speed() * dt * 0.5f;

    // Clamp to valid pan range
    if (engine->sky_offset < 0.0f)
        engine->sky_offset = 0.0f;
    if (engine->sky_offset > (float)SKY_PAN_RANGE)
        engine->sky_offset = (float)SKY_PAN_RANGE;

    // Read joystick and update car
    Joystick_Read(&joystick_cfg, &joystick_data);
    UserInput input = Joystick_GetInput(&joystick_data);
    car_update(&engine->car, input, dt_ms);
}

void race_engine_draw(race_engine_t *engine) {
    int16_t pan = (int16_t)engine->sky_offset;
    if (pan < 0) pan = 0;
    if (pan > SKY_PAN_RANGE) pan = SKY_PAN_RANGE;

    // Only redraw if pan has changed
    if (pan != engine->last_pan) {
        // Column offset via pointer
        uint8_t* sky_pointer = (uint8_t*)SKY + (SKY_CROP_START_ROW * SKY_SPRITE_WIDTH) + pan;
        LCD_Draw_Sprite(0, HORIZON_Y - SKY_CROP_NUM_ROWS, SKY_CROP_NUM_ROWS, SKY_SPRITE_WIDTH, sky_pointer);

        engine->last_pan = pan;
    }

    // Draw track
    track_draw(engine->car.pos_x);
    
    // Draw car
    car_draw(&engine->car);

    // Dirty rectangle to update stats
    LCD_Draw_Rect(0, 4, 240, 7, COLOUR_IDX_SKY, 1);

    // Lap time (top left):
    uint32_t lap_ms = track_get_lap_time_ms();
    uint32_t lap_secs = lap_ms / 1000;
    uint32_t lap_frac = (lap_ms % 1000) / 10;   // centiseconds
    char time_str[16];
    sprintf(time_str, "%02lu:%02lu", lap_secs, lap_frac);
    LCD_printString(time_str, 4, 4, COLOUR_IDX_KERB_WHITE, 1);

    // Lap counter (top right):
    char lap_str[16];
    sprintf(lap_str, "LAP:%u/%u", track_get_current_lap(), NUM_LAPS);
    LCD_printString(lap_str, 174, 4, COLOUR_IDX_KERB_WHITE, 1);

    // Fruit cointer (middle):
    char fruit_str[16];
    sprintf(fruit_str, "%ld/%ld", track_get_fruits_collected(), track_get_total_fruits());
    LCD_printString(fruit_str, 90, 4, COLOUR_IDX_KERB_WHITE, 1);

    LCD_Refresh(&cfg0);
}
