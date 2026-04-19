#ifndef GLOBALS_H
#define GLOBALS_H

#include <FastLED.h>


#ifndef ACTUAL_SLAVE_NUM
#define ACTUAL_SLAVE_NUM 10 // 不要修改這裡！ 請到platformio.ini去定義實際使用的slave數量!
#endif

#ifndef MAX_NUM_SLAVE
#define MAX_NUM_SLAVE 25 // 不要修改這裡！
#endif

#ifndef STORYMODE_0_TOTAL_SECONDS
#define STORYMODE_0_TOTAL_SECONDS 47 // 不要修改這裡！ 請到platformio_local.ini去定義
#endif

#ifndef STORYMODE_1_TOTAL_SECONDS
#define STORYMODE_1_TOTAL_SECONDS 176 // 不要修改這裡！ 請到platformio_local.ini去定義
#endif

#ifndef STORYMODE_2_TOTAL_SECONDS
#define STORYMODE_2_TOTAL_SECONDS 158 // 不要修改這裡！ 請到platformio_local.ini去定義
#endif

#ifndef STORYMODE_3_TOTAL_SECONDS
#define STORYMODE_3_TOTAL_SECONDS 158 // 不要修改這裡！ 請到platformio_local.ini去定義
#endif

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

#define PWM_FREQUENCY 60
#define PWM_MAX_BRIGHTNESS 4095

#define PWM_DISPATCH_FREQUENCY 20 // 20ms, 50Hz
#define PWM_I2C_FREQUENCY 400000
#define NORMAL_I2C_FREQUENCY 100000
#define OTA_I2C_FREQUENCY 400000
#define RGB_FREQUENCY 100

#define BUFFER_SIZE 64

#define USER_MODE 0
#define DEV_MODE 1
#define DISABLE 99

#define MILLI_AMPS 1600

//------------RGB LED PINS------------//
#define LED_PIN_RGB0 12

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

#ifndef NUM_LEDS_RGB0
#define NUM_LEDS_RGB0 200 // 不要修改這裡！ 請到platformio_local.ini去定義RGB燈珠數量!
#endif

#define NUM_LEDS_RGB0_STRIP1 15
#define NUM_LEDS_RGB0_STRIP2 15
#define NUM_LEDS_RGB0_STRIP3 15
#define NUM_LEDS_RGB0_STRIP4 15 

#define MAX_LEDS 180  // Or whatever your maximum LED count is

extern CRGB leds_RGB0[NUM_LEDS_RGB0];

extern CRGBSet strip1, strip2, strip3, strip4;

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

extern unsigned long startTime_footplate;
extern unsigned long startTime_axe;
extern unsigned long startTime_breath;
extern unsigned long startTime_turbine;

extern unsigned long startTime_mode0;
extern unsigned long startTime_mode1;
extern unsigned long startTime_mode2;
extern unsigned long startTime_mode3;
extern unsigned long startTime_modeDev;
extern unsigned long startTime_modeDemo;

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

  enum STATE_UART_TIMER {
      UART_TIMER_IDLE = 0x00,
      UART_TIMER_ACTIVE = 0x01,
      UART_TIMER_PAUSED = 0x02,
      UART_TIMER_EXPIRED = 0x03,
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

enum STATE_MODE_DEMO
{
    MODE_DEMO_INIT,
    MODE_DEMO_1,
    MODE_DEMO_1_IDLE,
    MODE_DEMO_2,
    MODE_DEMO_2_IDLE,
    MODE_DEMO_3,
    MODE_DEMO_3_IDLE,
    MODE_DEMO_4,
    MODE_DEMO_4_IDLE,
    MODE_DEMO_5,
    MODE_DEMO_5_IDLE,
    MODE_DEMO_6,
    MODE_DEMO_6_IDLE,
    MODE_DEMO_7,
    MODE_DEMO_7_IDLE,
    MODE_DEMO_8,
    MODE_DEMO_8_IDLE,
    MODE_DEMO_END
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
extern STATE_MODE_DEMO modeDemoState;

#endif // GLOBALS_H