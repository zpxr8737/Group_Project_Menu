#include "Car.h"
#include "LCD.h"
#include <math.h>
#include "sprites.h"

// Joystick control
extern Joystick_cfg_t joystick_cfg;

// Joystick data structure to hold readings
extern Joystick_t joystick_data;

enum direction { 
    LEFT = -1,
    RIGHT = 1,
    STRAIGHT = 0  
};

enum direction steer;

void car_init(Car_t *car) {
    car->pos_x            =  0.0f;
    car->curvature_offset =  0.0f;
}

void car_update(Car_t *car, UserInput input, uint32_t dt_ms) {
    // Steering input
    steer = 0;

    float dt = (float)dt_ms * 0.001f;

    // Only register input if joystick is pushed past the deadzone.
    if (input.magnitude > CAR_STEER_DEADZONE) {
        if (input.direction == W  || input.direction == NW || input.direction == SW) {
            steer = LEFT;   // left
        }
        else if (input.direction == E || input.direction == NE || input.direction == SE) {
            steer = RIGHT;    // right
        }
        else {steer = STRAIGHT;}
    }

    // Move at fixed speed
    car->pos_x += steer * CAR_STEER_SPEED * dt;

    // Curvature offset
    car->pos_x -= car->curvature_offset;
    car->curvature_offset = 0.0f;

    // Clamp position
    if (car->pos_x > CAR_MAX_POS_X) car->pos_x = CAR_MAX_POS_X;
    else if (car->pos_x < -CAR_MAX_POS_X) car->pos_x = -CAR_MAX_POS_X;
}

void car_draw(Car_t *car) {
    uint8_t *sprite;

    // Draw sprite based on lateral direction
    switch (steer) {
    case LEFT:
        sprite = (uint8_t *)CAR_LEFT;
        break;
    case RIGHT:
        sprite = (uint8_t *)CAR_RIGHT;
        break;
    case STRAIGHT:
        sprite = (uint8_t *)CAR_STRAIGHT;
        break;    
    }
    LCD_Draw_Sprite(CAR_SCREEN_CENTRE_X - CAR_SPRITE_W / 2, CAR_SCREEN_Y, CAR_SPRITE_H, CAR_SPRITE_W, sprite);
}