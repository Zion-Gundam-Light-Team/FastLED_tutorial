#ifndef GLOBALS_H
#define GLOBALS_H

#include <FastLED.h>

#ifndef WIFI_NAME // it is supposed to predefine in platformio.ini
#define WIFI_NAME "Angela_esp32" // Default value if not defined
#endif

#ifndef ACTUAL_SLAVE_NUM
#define ACTUAL_SLAVE_NUM 10 // 不要修改這裡！ 請到platformio.ini去定義實際使用的slave數量!
#endif

#ifndef MAX_NUM_SLAVE
#define MAX_NUM_SLAVE 25 // 不要修改這裡！
#endif

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

#define PWM_FREQUENCY 60
#define PWM_MAX_BRIGHTNESS 4095

#define MASTER_SLAVE_FREQUENCY 100000
#define RGB_FREQUENCY 100

#define BUFFER_SIZE 64

#define USER_MODE 0
#define DEV_MODE 1
#define DISABLE 99

#define MILLI_AMPS 1600

//------------RGB LED PINS------------//
#define LED_PIN_RGB1 1
#define LED_PIN_RGB2 2
#define LED_PIN_RGB3 3
#define LED_PIN_RGB4 4

//------------LED PINS------------//
//GPIO5 & 6 is reserved for PWM protocol channels
#define LED_PIN_7 7
#define LED_PIN_8 8
#define LED_PIN_9 9
#define LED_PIN_10 10 
#define LED_PIN_11 11
#define LED_PIN_16 16
#define LED_PIN_21 21
#define LED_PIN_33 33
#define LED_PIN_34 34
#define LED_PIN_35 35
#define LED_PIN_36 36
#define LED_PIN_37 37
#define LED_PIN_38 38
#define LED_PIN_39 39
#define LED_PIN_40 40
#define LED_PIN_41 41
#define LED_PIN_42 42
#define LED_PIN_45 45 
#define LED_PIN_47 47
#define LED_PIN_48 48

//------------PWM PINS------------//
#define PWM_LED_PIN_0 0
#define PWM_LED_PIN_1 1
#define PWM_LED_PIN_2 2
#define PWM_LED_PIN_3 3
#define PWM_LED_PIN_4 4
#define PWM_LED_PIN_5 5
#define PWM_LED_PIN_6 6
#define PWM_LED_PIN_7 7
#define PWM_LED_PIN_8 8
#define PWM_LED_PIN_9 9
#define PWM_LED_PIN_10 10
#define PWM_LED_PIN_11 11
#define PWM_LED_PIN_12 12
#define PWM_LED_PIN_13 13
#define PWM_LED_PIN_14 14
#define PWM_LED_PIN_15 15

#ifndef NUM_LEDS_RGB1
#define NUM_LEDS_RGB1 10 // 不要修改這裡！ 請到platformio.ini去定義RGB燈珠數量!
#endif
#ifndef NUM_LEDS_RGB2
#define NUM_LEDS_RGB2 10 // 不要修改這裡！ 請到platformio.ini去定義RGB燈珠數量!
#endif
#ifndef NUM_LEDS_RGB3
#define NUM_LEDS_RGB3 10 // 不要修改這裡！ 請到platformio.ini去定義RGB燈珠數量!
#endif
#ifndef NUM_LEDS_RGB4
#define NUM_LEDS_RGB4 10 // 不要修改這裡！ 請到platformio.ini去定義RGB燈珠數量!
#endif

#define MAX_LEDS 180  // Or whatever your maximum LED count is

extern CRGB leds_RGB1[NUM_LEDS_RGB1];
extern CRGB leds_RGB2[NUM_LEDS_RGB2];
extern CRGB leds_RGB3[NUM_LEDS_RGB3];
extern CRGB leds_RGB4[NUM_LEDS_RGB4];

extern char buffer[1024];

extern uint8_t currentModeId;
extern bool runStoryCompleted;
extern bool enableRunStory;

extern uint8_t UARTVideoData;
extern bool UARTSendImmediate;

extern uint8_t currentPaletteIndex;

extern uint8_t isRepeatMode;
extern uint8_t gHue;
extern uint8_t brightness;

extern uint8_t cooling;  // suggested range 20-100
extern uint8_t sparking; // suggested range 50-200

extern int SSstate;

extern const char *WIFI_PASSWORD;
extern bool wifiSetUp;
extern int ApTimeout;
extern bool wifiResponse;
extern bool wifiInitialized;

extern unsigned long startTime_footplate;
extern unsigned long startTime_axe;
extern unsigned long startTime_breath;
extern unsigned long startTime_turbine;

extern unsigned long startTime_mode0;
extern unsigned long startTime_mode1;
extern unsigned long startTime_mode2;
extern unsigned long startTime_mode3;

// Timeout tracking for dev mode
extern unsigned long lastMasterPollTime;
extern const unsigned long MASTER_TIMEOUT_MS;
extern bool isInDevMode;
extern unsigned long lastCountdownPrint;

enum STATE_UART_VIDEO
{
    UART_VIDEO_IDLE = 0xEE,
    UART_VIDEO_PLAY_1 = 0x01,
    UART_VIDEO_PLAY_2 = 0x02,
    UART_VIDEO_PLAY_3 = 0x03,
    UART_VIDEO_PLAY_4 = 0x04,
    UART_VIDEO_STOP = 0xAA,
};

enum STATE_FOOTPLATE
{
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

enum STATE_AXE
{
    AXE_INIT,
    AXE_SWIPEIN,
    AXE_WHITELIGHT_SWIPE,
    AXE_SWIPEOUT,
    AXE_END
};

enum STATE_BREATH
{
    BREATH_INIT,
    BREATH_SWIPEIN,
    BREATH_MAIN,
    BREATH_FADE_OUT
};

enum STATE_STORING_ENERGY
{
    STORING_ENERGY_INIT = 0,
    STORING_ENERGY_FILL,
    STORING_ENERGY_FALLBACK,
    STORING_ENERGY_HOLD,
    STORING_ENERGY_END,
};

enum STATE_GUNFIRE
{
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

enum STATE_TURBINE
{
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

enum STATE_MODE_0
{
    MODE_0_INIT,
    MODE_0_EYE,
    MODE_0_START_FLASH,
    MODE_0_FLASH,
    MODE_0_ONE_OVER_FOUR,
    MODE_0_TWO_OVER_FOUR,
    MODE_0_THREE_OVER_FOUR,
    MODE_0_ALL,
    MODE_0_FADEOUT,
    MODE_0_END
};

enum STATE_MODE_1
{
    MODE_1_INIT,
    MODE_1_EYE,
    MODE_1_START,
    MODE_1_CONTINUE,
    MODE_1_FADEOUT,
    MODE_1_END
};

enum STATE_MODE_2
{
    MODE_2_INIT,
    MODE_2_EYE,
    MODE_2_START,
    MODE_2_CONTINUE,
    MODE_2_FADEOUT,
    MODE_2_END
};

enum STATE_MODE_3
{
    MODE_3_INIT,
    MODE_3_EYE,
    MODE_3_START,
    MODE_3_CONTINUE,
    MODE_3_FADEOUT,
    MODE_3_END
};

enum STATE_MODE_DEV
{
    MODE_DEV_START
};

extern STATE_FOOTPLATE footPlateState;
extern STATE_AXE axeState;
extern STATE_BREATH breathState;
extern STATE_STORING_ENERGY storingEnergyState;
extern STATE_TURBINE turbineState;
extern STATE_MODE_0 mode0State;
extern STATE_MODE_1 mode1State;
extern STATE_MODE_2 mode2State;
extern STATE_MODE_3 mode3State;
extern STATE_MODE_DEV modeDevState;

#endif // GLOBALS_H