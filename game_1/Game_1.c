#include "Game_1.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "Joystick.h"
#include "RaceEngine.h"
#include "Track.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include "sprites.h"
#include <string.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control
extern Joystick_cfg_t joystick_cfg; // Joystick control
// Joystick data structure to hold readings
extern Joystick_t joystick_data;
race_engine_t race_engine;

// Frame rate for this game (in milliseconds)
#define GAME1_FRAME_TIME_MS 30  // ~33 FPS

MenuState Game1_Run(void) {
    // Play a brief startup sound
    buzzer_tone(&buzzer_cfg, 1000, 30);  // 1kHz at 30% volume
    HAL_Delay(50);  // Brief beep duration
    buzzer_off(&buzzer_cfg);  // Stop the buzzer
    
    MenuState exit_state = MENU_STATE_HOME;  // Default: return to menu
    
    // Race engine init
    race_engine_init(&race_engine);    

    // Game's own loop - runs until exit condition
    while (track_is_race_complete() != 1) {
        uint32_t frame_start = HAL_GetTick();
        
        // Read input
        Input_Read();
        
        // Check if button was pressed to return to menu
        if (current_input.btn3_pressed) {
            exit_state = MENU_STATE_HOME;
            break;  // Exit game loop
        }

        // Update game logic
        uint8_t btn2_held = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == GPIO_PIN_RESET);
        race_engine_update(&race_engine, btn2_held);

        // Render to LCD
        race_engine_draw(&race_engine);
    
        // Frame timing - wait for remainder of frame time
        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < GAME1_FRAME_TIME_MS)
            HAL_Delay(GAME1_FRAME_TIME_MS - frame_time);
        }

        while (track_is_race_complete()) {
        // Format best lap time
        uint32_t best_ms = track_get_best_lap_ms();
        uint32_t best_secs = best_ms / 1000;
        uint32_t best_frac = (best_ms % 1000) / 10;

        // Format fruit count
        int32_t collected = track_get_fruits_collected();
        int32_t total = track_get_total_fruits();

        // Build strings
        char best_lap_str[32];
        char fruit_str[32];
        char title_str[] = "RACE COMPLETE";
        char exit_str[] = "BTN3 TO EXIT";

        sprintf(best_lap_str, "BEST LAP: %02lu:%02lu", best_secs, best_frac);
        sprintf(fruit_str, "FRUITS: %ld/%ld", collected, total);

        LCD_Fill_Buffer(12);   // colour index 12 = background

        // RACE COMPLETE
        uint16_t title_x = (SCREEN_WIDTH - (uint16_t)(strlen(title_str) * 6 * 2)) / 2;
        LCD_printString(title_str, title_x, 70, 10, 2);

        // Best lap
        uint16_t lap_x = (SCREEN_WIDTH - (uint16_t)(strlen(best_lap_str) * 6 * 2)) / 2;
        LCD_printString(best_lap_str, lap_x, 110, 10, 2);

        // Fruits
        uint16_t fruit_x = (SCREEN_WIDTH - (uint16_t)(strlen(fruit_str) * 6 * 2)) / 2;
        LCD_printString(fruit_str, fruit_x, 140, 10, 2);

        // Exit prompt
        uint16_t exit_x = (SCREEN_WIDTH - (uint16_t)(strlen(exit_str) * 6 * 2)) / 2;
        LCD_printString(exit_str, exit_x, 190, 10, 2);

        LCD_Refresh(&cfg0);
        
        // Wait for BTN3 press to exit to menu
        Input_Read();
        if (current_input.btn3_pressed) {
            exit_state = MENU_STATE_HOME;
            break;
        }
        HAL_Delay(50);
    }

    LCD_Set_Palette(PALETTE_DEFAULT);

    return exit_state;  // Tell main where to go next
}