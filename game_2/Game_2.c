#include "Game_2.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "Buzzer.h"
#include "Joystick.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "sprites.h"   // fruit sprites (in shared folder)
 
extern ST7789V2_cfg_t cfg0;
extern Buzzer_cfg_t buzzer_cfg;
extern Joystick_cfg_t joystick_cfg;
extern Joystick_t joystick_data;
extern const uint8_t CAR_STRAIGHT[64][64];
 
// game state
static int16_t car_x = SCREEN_WIDTH / 2;
static int16_t car_y = SCREEN_HEIGHT - 32;
static int16_t car_speed = 5;
 
static Bullet bullets[MAX_BULLETS];
static Fruit fruits[MAX_FRUITS];
static uint32_t score = 0;
 
// init fruits
static void InitFruits(void) {
    for (int i = 0; i < MAX_FRUITS; i++) {
        fruits[i].x = rand() % (SCREEN_WIDTH - 20) + 10;
        fruits[i].y = -(rand() % 200);
        fruits[i].old_x = fruits[i].x;
        fruits[i].old_y = fruits[i].y;
        fruits[i].active = 1;
        fruits[i].type = rand() % 3;
    }
}
 
// fire bullet
static void FireBullet(void) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = car_x;
            bullets[i].y = car_y - 10;
            bullets[i].active = 1;
 
            buzzer_tone(&buzzer_cfg, 1500, 20);
            HAL_Delay(10);
            buzzer_off(&buzzer_cfg);
            break;
        }
    }
}
 
// collision check
static uint8_t CheckCollision(Bullet *b, Fruit *f) {
    if (!b->active || !f->active) return 0;
 
    if (b->x >= f->x - 8 && b->x <= f->x + 8 &&
        b->y >= f->y - 8 && b->y <= f->y + 8) {
        return 1;
    }
    return 0;
}
 
// main game loop
MenuState Game2_Run(void)
{
    car_x = SCREEN_WIDTH / 2;
    score = 0;
 
    InitFruits();
 
    for (int i = 0; i < MAX_BULLETS; i++)
        bullets[i].active = 0;
 
    LCD_Set_Palette(PALETTE_CUSTOM);
 
    MenuState exit_state = MENU_STATE_HOME;
 
    //  shooting cooldown (NEW)
    uint32_t last_shot_time = 0;
    const uint32_t shoot_delay = 200; // ms
 
    while (1)
    {
        uint32_t frame_start = HAL_GetTick();
 
        Input_Read();
        Joystick_Read(&joystick_cfg, &joystick_data);
 
        //  exit
        if (current_input.btn2_pressed)
        {
            exit_state = MENU_STATE_HOME;
            break;
        }
 
        //  movement
        if (joystick_data.direction == W)
            car_x -= car_speed;
 
        if (joystick_data.direction == E)
            car_x += car_speed;
 
        if (car_x < 32) car_x = 32;
        if (car_x > SCREEN_WIDTH - 32) car_x = SCREEN_WIDTH - 32;
 
        //  smooth shooting (FIXED)
        if (current_input.btn3_pressed &&
            (HAL_GetTick() - last_shot_time > shoot_delay))
        {
            FireBullet();
            last_shot_time = HAL_GetTick();
        }
 
        //  update bullets
        for (int i = 0; i < MAX_BULLETS; i++)
        {
            if (bullets[i].active)
            {
                bullets[i].y -= 8;
 
                if (bullets[i].y < 0)
                    bullets[i].active = 0;
            }
        }
 
        //  update fruits
        for (int i = 0; i < MAX_FRUITS; i++)
        {
            if (fruits[i].active)
            {
                fruits[i].old_x = fruits[i].x;
                fruits[i].old_y = fruits[i].y;
 
                fruits[i].y += 3;
 
                if (fruits[i].y > SCREEN_HEIGHT)
                {
                    fruits[i].y = -20;
                    fruits[i].x = rand() % (SCREEN_WIDTH - 20) + 10;
                    fruits[i].type = rand() % 3;
                }
            }
        }
 
        //  collisions
        for (int i = 0; i < MAX_BULLETS; i++)
        {
            for (int j = 0; j < MAX_FRUITS; j++)
            {
                if (CheckCollision(&bullets[i], &fruits[j]))
                {
                    bullets[i].active = 0;
 
                    fruits[j].active = 0;
                    score += 10;
 
                    buzzer_tone(&buzzer_cfg, 2000, 30);
                    HAL_Delay(20);
                    buzzer_off(&buzzer_cfg);
 
                    // respawn fruit
                    fruits[j].x = rand() % (SCREEN_WIDTH - 20) + 10;
                    fruits[j].y = -(rand() % 200);
                    fruits[j].type = rand() % 3;
                    fruits[j].active = 1;
                }
            }
        }
 
        //  clear buffer
        LCD_Fill_Buffer(0);
 
//  DRAW CAR SPRITE with correct 64x64 sprite
        LCD_Draw_Sprite(
            car_x - 32,
            car_y - 32,
            64,
            64,
            (const uint8_t*)CAR_STRAIGHT
        );
 
        //  bullets
        for (int i = 0; i < MAX_BULLETS; i++)
        {
            if (bullets[i].active)
            {
                LCD_printString("|", bullets[i].x, bullets[i].y, 15, 2);
            }
        }
 
        //  fruits
        for (int i = 0; i < MAX_FRUITS; i++)
        {
            if (!fruits[i].active)
                continue;
 
            uint8_t* sprite = 0;
 
            switch (fruits[i].type)
            {
                case 0: sprite = (uint8_t*)appleSprite; break;
                case 1: sprite = (uint8_t*)orangeSprite; break;
                case 2: sprite = (uint8_t*)watermelonSprite; break;
            }
 
            LCD_Draw_Sprite(
                fruits[i].x - 8,
                fruits[i].y - 8,
                16,
                16,
                sprite
            );
        }
 
        //  score
        char score_str[32];
        sprintf(score_str, "Score: %lu", (unsigned long)score);
        LCD_printString(score_str, 10, 10, 15, 2);
 
        LCD_printString("Left/Right: Move", 10, SCREEN_HEIGHT - 40, 15, 1);
        LCD_printString("BTN3: Shoot", 10, SCREEN_HEIGHT - 25, 15, 1);
        LCD_printString("BTN2: Exit", 10, SCREEN_HEIGHT - 10, 15, 1);
 
        //  refresh screen
        LCD_Refresh(&cfg0);
 
        // ⏱ frame control
        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < GAME2_FRAME_TIME_MS)
        {
            HAL_Delay(GAME2_FRAME_TIME_MS - frame_time);
        }
    }
 
    LCD_Set_Palette(PALETTE_DEFAULT);
    return exit_state;
}
