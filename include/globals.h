#ifndef GLOBALS_H
#define GLOBALS_H

#include <FastLED.h>

#define PWM_FREQUENCY 1000
#define PWM_MAX_BRIGHTNESS 4095
#define RGB_FREQUENCY 10

#define MAX_LEDS 180  // Or whatever your maximum LED count is

#define NUM_RGB1 10
#define NUM_RGB2 10
#define NUM_RGB3 10

//------------PWM I2C-------------//
#define PIN_PWM_SDA 5
#define PIN_PWM_SCL 6
//-------------RGB----------------//
#define PIN_RGB1 1
#define PIN_RGB2 2
#define PIN_RGB3 3

//---------------LED--------------//
#define PIN_LED_5 5
#define PIN_LED_6 6
#define PIN_LED_7 7
#define PIN_LED_8 8
#define PIN_LED_9 9
#define PIN_LED_10 10
#define PIN_LED_11 11

//------------PWM PINS------------//
#define PIN_PWM_0 0
#define PIN_PWM_1 1
#define PIN_PWM_2 2
#define PIN_PWM_3 3
#define PIN_PWM_4 4
#define PIN_PWM_5 5
#define PIN_PWM_6 6
#define PIN_PWM_7 7
#define PIN_PWM_8 8
#define PIN_PWM_9 9
#define PIN_PWM_10 10
#define PIN_PWM_11 11
#define PIN_PWM_12 12
#define PIN_PWM_13 13
#define PIN_PWM_14 14
#define PIN_PWM_15 15

extern CRGB leds_RGB1[NUM_RGB1];
extern CRGB leds_RGB2[NUM_RGB2];
extern CRGB leds_RGB3[NUM_RGB3];

extern uint8_t colorIndex_RGB1[NUM_RGB1];
extern uint8_t colorIndex_RGB2[NUM_RGB2];

extern uint8_t currentModeId;
extern uint8_t ledFlashCount;

extern uint8_t isRepeatMode;
extern uint8_t gHue;
extern uint8_t brightness;

extern uint8_t cooling;   // suggested range 20-100
extern uint8_t sparking;  // suggested range 50-200

extern unsigned long startTime_footplate;
extern unsigned long startTime_axe;
extern unsigned long startTime_breath;
extern unsigned long startTime_turbine;
extern unsigned long startTime_mode0;
extern unsigned long startTime_mode1;
extern unsigned long startTime_mode2;
extern unsigned long startTime_mode3;
extern unsigned long startTime_modeDemo;

enum STATE_FOOTPLATE {
    FOOTPLATE_INIT,
    FOOTPLATE_FADEIN,
    FOOTPLATE_FLASH1,
    FOOTPLATE_FLASH2,
    FOOTPLATE_FLASH3,
    FOOTPLATE_FLASH4,
    FOOTPLATE_FLASH5,
    FOOTPLATE_FLASH6,
    FOOTPLATE_FLASH7,
    FOOTPLATE_FLASH8,
    FOOTPLATE_KEEP,
    FOOTPLATE_FADEOUT,
    FOOTPLATE_END
};

enum STATE_AXE {
    AXE_INIT,
    AXE_SWIPEIN,
    AXE_WHITELIGHT_SWIPE,
    AXE_SWIPEOUT,
    AXE_END
};

enum STATE_BREATH { BREATH_INIT, BREATH_SWIPEIN, BREATH_MAIN, BREATH_FADE_OUT };

enum STATE_STORING_ENERGY {
    STORING_ENERGY_INIT = 0,
    STORING_ENERGY_FILL,
    STORING_ENERGY_FALLBACK,
    STORING_ENERGY_END,
};

enum STATE_TURBINE {
    TURBINE_INIT,
    TURBINE_WHITE_FLASH1,
    TURBINE_OFF_KEEP,
    TURBINE_RAINBOW_ROTATE,
    TURBINE_WHITE_ROTATE,
    TURBINE_WHITE_FLASH2,
    TURBINE_RAINBOW_FLASH,
    TURBINE_RAIBNOW_RED_FADEOUT,
    TURBINE_END
};

enum STATE_VENT { VENT_INIT, VENT_MAIN, VENT_FADE_OUT };

enum STATE_CYBER { CYBER_INIT, CYBER_MAIN, CYBER_FADE_OUT };

enum STATE_MODE_0 { MODE_0_INIT, MODE_0_MAIN, MODE_0_END };

enum STATE_MODE_1 {
    MODE_1_INIT,
    MODE_1_START,
    MODE_1_CONTINUE,
    MODE_1_FADEOUT,
    MODE_1_END
};

enum STATE_MODE_DEMO { MODE_DEMO_INIT, MODE_DEMO_MAIN, MODE_DEMO_END };

enum STATE_GUNFIRE {
    GUNFIRE_INIT = 0,
    GUNFIRE_FADEIN,
    GUNFIRE_COLOR_FLASH1,
    GUNFIRE_COLOR_FLASH2,
    GUNFIRE_COLOR_FLASH3,
    GUNFIRE_LED_KEEP1,
    GUNFIRE_LED_KEEP2,
    GUNFIRE_LED_FREEZE,
    GUNFIRE_FLASH1,
    GUNFIRE_FLASH2,
    GUNFIRE_BLUE_TO_RED,
    GUNFIRE_FADEOUT,
    GUNFIRE_KEEP2,
    GUNFIRE_IDLE

};

extern unsigned long stateStartTime;
extern unsigned long startTime_gunfire;
extern STATE_FOOTPLATE footPlateState;
extern STATE_AXE axeState;
extern STATE_BREATH breathState;
extern STATE_STORING_ENERGY storingEnergyState;
extern STATE_TURBINE turbineState;
extern STATE_VENT ventState;
extern STATE_CYBER cyberLight;
extern STATE_MODE_0 mode0State;
extern STATE_MODE_1 mode1State;
extern STATE_MODE_DEMO modeDemoState;
extern STATE_GUNFIRE gunfireState;
#endif  // GLOBALS_H