#include <FastLED.h>
#include "../include/globals.h"

char buffer[1024];

CRGB leds_RGB1[NUM_LEDS_RGB1];
CRGB leds_RGB2[NUM_LEDS_RGB2];
CRGB leds_RGB3[NUM_LEDS_RGB3];
CRGB leds_RGB4[NUM_LEDS_RGB4];

uint8_t currentModeId = DISABLE;
uint8_t currentPaletteIndex = 0;
bool runStoryCompleted = false;
bool enableRunStory = true;

uint8_t UARTVideoData = UART_VIDEO_IDLE;
bool UARTSendImmediate = false;

uint8_t isRepeatMode = 0;
uint8_t gHue = 0;
uint8_t brightness = 30;

uint8_t cooling = 50;   // suggested range 20-100
uint8_t sparking = 120; // suggested range 50-200

const char *WIFI_PASSWORD = "12345678";
bool wifiSetUp = false;
int ApTimeout = 120;
bool wifiResponse = false;
bool wifiInitialized = false;

// Timeout tracking for dev mode
unsigned long lastMasterPollTime = 0;
const unsigned long MASTER_TIMEOUT_MS = 10000;
bool isInDevMode = false;
unsigned long lastCountdownPrint = 0;

unsigned long startTime_footplate = 0;
unsigned long startTime_axe = 0;
unsigned long startTime_breath = 0;
unsigned long startTime_turbine = 0;
unsigned long startTime_mode0 = 0;
unsigned long startTime_mode1 = 0;
unsigned long startTime_mode2 = 0;
unsigned long startTime_mode3 = 0;

STATE_BREATH breathState = BREATH_INIT;
STATE_FOOTPLATE footPlateState = FOOTPLATE_INIT;
STATE_AXE axeState = AXE_INIT;
STATE_STORING_ENERGY storingEnergyState = STORING_ENERGY_INIT;
STATE_TURBINE turbineState = TURBINE_INIT;
STATE_MODE_0 mode0State = MODE_0_INIT;
STATE_MODE_1 mode1State = MODE_1_INIT;
STATE_MODE_2 mode2State = MODE_2_INIT;
STATE_MODE_3 mode3State = MODE_3_INIT;
STATE_MODE_DEV modeDevState = MODE_DEV_START;

STATE_UART_VIDEO uartVideoState = UART_VIDEO_IDLE;