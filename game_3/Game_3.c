#include "Game_3.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "Joystick.h"
#include "stm32l4xx_hal.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control
extern Joystick_cfg_t joystick_cfg;
extern Joystick_t joystick_data;


//game constants
#define screenWidth 240
#define screenHeight 240
#define maxTargets 4
#define baseSpeed 120.0f
#define comboTimeMS 1500
#define gameFrameMS 16
#define longPressMS 500

//fruit types
typedef enum{
    fruitApple,
    fruitOrange,
    fruitWatermelon
} FruitType;


// game structures
typedef enum{
stateMenu,
statePlaying,
statePaused,
StateGameOver
} GameState;

typedef struct{
float x;
float y;
} Vector2D;

typedef struct{
Vector2D pos;
Vector2D vel;
uint8_t active;
uint32_t hitFlashEnd;
uint8_t type;
uint8_t radius;
} Target_t;

typedef struct{
GameState state;
Target_t targets[maxTargets];
uint8_t targetCount;
Vector2D crosshair;
uint16_t score;
uint8_t lives;
uint8_t combo;
uint32_t lastHitTime;
float gameSpeed;
float sensitivity;
uint16_t highScore;
uint32_t ledFlashEnd;
} TargetGameEngine_t;

//sprites

static const uint8_t appleSprite[16][16]={
    {255,255,255,255,255,2,2,2,2,2,2,255,255,255,255,255},
    {255,255,255,2,2,2,2,2,2,2,2,2,2,255,255,255},
    {255,255,2,2,2,2,2,2,2,2,2,2,2,2,255,255},
    {255,2,2,2,2,2,2,2,2,2,2,2,2,2,2,255},
    {255,2,2,2,2,2,2,2,2,2,2,2,2,2,2,255},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {255,2,2,2,2,2,2,2,2,2,2,2,2,2,2,255},
    {255,2,2,2,2,2,2,2,2,2,2,2,2,2,2,255},
    {255,255,2,2,2,2,2,2,2,2,2,2,2,2,255,255},
    {255,255,255,2,2,2,2,2,2,2,2,2,2,255,255,255},
    {255,255,255,255,255,2,2,2,2,2,2,255,255,255,255,255},
    {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255}
};

static const uint8_t orangeSprite[16][16]{
    {255,255,255,255,255,4,4,4,4,4,4,255,255,255,255,255},
    {255,255,255,4,4,4,4,4,4,4,4,4,4,255,255,255},
    {255,255,4,4,4,4,4,4,4,4,4,4,4,4,255,255},
    {255,4,4,4,4,4,4,4,4,4,4,4,4,4,4,255},
    {255,4,4,4,4,4,4,4,4,4,4,4,4,4,4,255},
    {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
    {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
    {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
    {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
    {255,4,4,4,4,4,4,4,4,4,4,4,4,4,4,255},
    {255,4,4,4,4,4,4,4,4,4,4,4,4,4,4,255},
    {255,255,4,4,4,4,4,4,4,4,4,4,4,4,255,255},
    {255,255,255,4,4,4,4,4,4,4,4,4,4,255,255,255},
    {255,255,255,255,255,4,4,4,4,4,4,255,255,255,255,255},
    {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255}
    
};

static const uint8_t watermelonSprite[16][16] ={
    {255,255,255,255,255,3,3,3,3,3,3,255,255,255,255,255},
    {255,255,255,3,3,3,3,3,3,3,3,3,3,255,255,255},
    {255,255,3,3,3,3,3,3,3,3,3,3,3,3,255,255},
    {255,3,3,3,3,3,3,3,3,3,3,3,3,3,3,255},
    {255,3,3,3,3,3,3,3,3,3,3,3,3,3,3,255},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {255,3,3,3,3,3,3,3,3,3,3,3,3,3,3,255},
    {255,3,3,3,3,3,3,3,3,3,3,3,3,3,3,255},
    {255,255,3,3,3,3,3,3,3,3,3,3,3,3,255,255},
    {255,255,255,3,3,3,3,3,3,3,3,3,3,255,255,255},
    {255,255,255,255,255,3,3,3,3,3,3,255,255,255,255,255},
    {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255}

};

static const uint8_t crosshairSprite[16][16] = {
    {255,255,255,255,255,255,255,2,2,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,2,2,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,2,2,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,2,2,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,2,2,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,2,2,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,2,2,255,255,255,255,255,255,255},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {255,255,255,255,255,255,255,2,2,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,2,2,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,2,2,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,2,2,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,2,2,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,2,2,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,2,2,255,255,255,255,255,255,255}
};


//global initialisation
static TargetGameEngine_t game;

// fruit properties

static uint8_t getFruitRadius(uint8_t type){
    switch(type){
        case fruitApple: return 8;
        case fruitOrange: return 10;
        case fruitWatermelon: return 14;
        default: return 10;
    }
}

static uint16_t getFruitScore(uint8_t type){
    switch(type){
        case fruitApple: return 10;
        case fruitOrange: return 15;
        case fruitWatermelon: return 25;
        default: return 10;
    }
}

static float getFruitFleeForce(uint8_t type){
    switch(type){
        case fruitApple: return 150.0f;
        case fruitOrange: return 200.0f;
        case fruitWatermelon: return 300.0f;
        default: return 200.0f;
    }
    
}

static const uint8_t* getFruitSprite(uint8_t type){
    switch(type){
        case fruitApple: return (uint8_t*)appleSprite;
        case fruitOrange: return (uint8_t*)orangeSprite;
        case fruitWatermelon: return (uint8_t*)watermelonSprite;
        default: return (uint8_t*)appleSprite;
    }
    
}

static uint16_t getFruitHitSound(uint8_t type, uint8_t combo){
    uint16_t base;
    switch(type){
        case fruitApple: base = 900; break;
        case fruitOrange: base = 1100; break;
        case fruitWatermelon: base = 700; break;
        default: base = 1000; break;
    }
    uint16_t add = (combo>5 ? 5: combo) * 50
    return base + add;
}

//helper functions

static uint32_t randomGen(uint32_t max){
    static uint32_t seed = 12345;
    seed = seed * 1103515245 + 12345;
    return (seed >> 16)%max;
}

static void startLedFlash(uint32_t durationMS){
    game.ledFlashEnd = HAL_GetTick()+durationMS;
    PWM_SetDuty(&pwm_cfg,80);
    }

static void updateLedFlash(void){
    if(game.ledFlashEnd && HAL_GetTick() >= game.ledFlashEnd){
    PWM_SetDuty(&pwm_cfg,0);
    game.ledFlashEnd=0;
    }

}

static void playHitSound(uint8_t combo){
    uint16_t frequency = 1000 + (combo*100);
    if (frequency>3000){
        frequency=3000;
    }    
    buzzer_tone(&buzzer_cfg,frequency,30);
    HAL_Delay(30);
    buzzer_off(&buzzer_cfg);
    
}

static void playMissSound(void){
    buzzer_tone(&buzzer_cfg,300,30);
    HAL_Delay(50);
    buzzer_off(&buzzer_cfg);
}

static void playGameOverSound(void){
    buzzer_tone(&buzzer_cfg,150,30);
    HAL_Delay(300);
    buzzer_off(&buzzer_cfg);
    buzzer_tone(&buzzer_cfg,100,30);
    HAL_Delay(500);
    buzzer_off(&buzzer_cfg);
    
}

// core gameplay

static void addTarget(void){
    
    
}

static void updateGame(float deltaSec){
    if(game.state != statePlaying){
        return;
    }
    for(int i =0; i<maxTargets;i++){
        if(!game.targets[i].active){
            continue;
        }
        //movements
        game.targets[i].pos.x += game.targets[i].vel.x *deltaSec;
        game.targets[i].pos.y += game.targets[i].vel.y *  deltaSec;

        //bounce
        if(game.targets[i].pos.x < game.targets[i].radius){
            game.targets[i].pos.x = game.targets[i].radius;
            game.targets[i].vel.x = -game.targets[i].vel.x;
        }
        if(game.targets[i].pos.x > screenWidth - game.targets[i].radius){
            game.targets[i].pos.x = screenWidth - game.targets[i].radius;
            game.targets[i].vel.x = -game.targets[i].vel.x;
            
        }
        if(game.targets[i].pos.y < game.targets[i].radius){
            game.targets[i].pos.y = game.targets[i].radius;
            game.targets[i].vel.y = -game.targets[i].vel.y;
            
        }
        if(game.targets[i].pos.y > screenHeight - game.targets[i].radius){
            game.targets[i].pos.y = screenHeight - game.targets[i].radius;
            game.targets[i].vel.y = -game.targets[i].vel.y;
            
        }
        //fleeing
        float dx = game.targets[i].pos.x - game.crosshair.x;
        float dy = game.targets[i].pos.y - game.crosshair.y;
        float distance = sqrtf(dx*dx + dy*dy);
        float fleeDistance = (game.targets[i].type == fruitWatermelon) ? 70.0f: 50.0f;
        if(distance<fleeDistance && distance > 0.1f){
            float awayx=dx/distance;
            float awayy = dy/distance;
            float force = getFruitFleeForce(game.targets[i].type);
            game.targets[i].vel.x +=awayx*force* deltaSec;
            game.targets[i].vel.y =awayy*force*deltaSec;
            float maxSpeed = baseSpeed * game.gameSpeed * 2.0f;
            float speed = sqrtf(game.targets[i].vel.x*game.targets[i].vel.x+game.targets[i].vel.y*game.targets[i].vel.y);
            if(speed>maxSpeed){
                game.targets[i].vel.x =  game.targets[i].vel.x/speed*maxSpeed;
                game.targets[i].vel.y =  game.targets[i].vel.y/speed*maxSpeed;
            }
        }
    }

}


