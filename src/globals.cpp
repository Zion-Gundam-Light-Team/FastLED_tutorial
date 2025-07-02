#include <FastLED.h>
#include "../include/globals.h"

CRGB leds_RGB1[NUM_RGB1];
CRGB leds_RGB2[NUM_RGB2];
CRGB leds_RGB3[NUM_RGB3];
CRGB leds_RGB4[NUM_RGB4];
CRGB leds_RGB13[NUM_RGB13];
CRGB leds_RGB14[NUM_RGB14];
CRGB leds_RGB15[NUM_RGB15];
CRGB leds_RGB16[NUM_RGB16];
CRGB leds_RGB17[NUM_RGB17];
CRGB leds_RGB18[NUM_RGB18];

uint8_t colorIndex_RGB1[NUM_RGB1];
uint8_t colorIndex_RGB2[NUM_RGB2];
uint8_t colorIndex_RGB3[NUM_RGB3];
uint8_t colorIndex_RGB4[NUM_RGB4];

uint8_t currentModeId = 0;

uint8_t ledFlashCount = 0;

uint8_t isRepeatMode = 0;
uint8_t gHue = 0;
uint8_t brightness = 30;

uint8_t cooling = 50;   // suggested range 20-100
uint8_t sparking = 120; // suggested range 50-200

unsigned long stateStartTime = 0;
unsigned long startTime_footplate = 0;
unsigned long startTime_axe = 0;
unsigned long startTime_breath = 0;
unsigned long startTime_turbine = 0;
unsigned long startTime_mode0 = 0;
unsigned long startTime_mode1 = 0;
unsigned long startTime_mode2 = 0;
unsigned long startTime_mode3 = 0;
unsigned long startTime_modeDemo = 0;
unsigned long startTime_gunfire = 0;

STATE_BREATH breathState = BREATH_INIT;
STATE_FOOTPLATE footPlateState = FOOTPLATE_INIT;
STATE_AXE axeState = AXE_INIT;
STATE_STORING_ENERGY storingEnergyState = STORING_ENERGY_INIT;
STATE_TURBINE turbineState = TURBINE_INIT;
STATE_VENT ventState = VENT_INIT;
STATE_CYBER cyberLight = CYBER_INIT;

STATE_MODE_0 mode0State = MODE_0_INIT;
STATE_MODE_1 mode1State = MODE_1_INIT;
STATE_MODE_2 mode2State = MODE_2_INIT;
STATE_MODE_3 mode3State = MODE_3_INIT;
STATE_MODE_DEMO modeDemoState = MODE_DEMO_INIT;
STATE_GUNFIRE gunfireState = GUNFIRE_INIT;
