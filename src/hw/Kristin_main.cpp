#include <Arduino.h>
#include <FastLED.h>

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define PIN_RGB1 15
#define PIN_RGB2 14
#define PIN_RGB3 13
#define NUM_RGB1 10
#define NUM_RGB2 10
#define NUM_RGB3 10

CRGB leds_RGB1[NUM_RGB1];
CRGB leds_RGB2[NUM_RGB2];
CRGB leds_RGB3[NUM_RGB3];
uint8_t startIndex = 0;
uint8_t randomIndex = 0;
// Q3 int LED_PIN[] = {1,2,3,4,5,6};
int leds = 15;
int leds2 = 14;
int leds3 = 13;
int NUM_LEDS = 10;
uint8_t speed = 20;
uint8_t hue = 0;
uint8_t chance = 50;
uint8_t currentBrightness = 0;
uint8_t colorIndex = 0;

DEFINE_GRADIENT_PALETTE(heatmap_gp){
    0, 0, 189, 255,      // light blue
    128, 0, 91, 255,     // blue
    200, 47, 237, 83,    // green
    255, 255, 255, 255}; // white

CRGBPalette16 heatmapPalette = heatmap_gp;

void setup()
{
    Serial.begin(115200);

    FastLED.addLeds<LED_TYPE, PIN_RGB1, COLOR_ORDER>(leds_RGB1, NUM_RGB1);
    FastLED.addLeds<LED_TYPE, PIN_RGB2, COLOR_ORDER>(leds_RGB2, NUM_RGB2);
    FastLED.addLeds<LED_TYPE, PIN_RGB3, COLOR_ORDER>(leds_RGB3, NUM_RGB3);
    // Q3
    // for (int i = 0; i < 6; i++){
    // pinMode(LED_PIN[i], OUTPUT);
    // }
}

// Q5
void dynamicRainbow(CRGB *leds3, int NUM_LEDS3, uint8_t *hue, uint8_t speed)
{
    CRGB color = CHSV(*hue, 255, 255);
    fill_rainbow(leds3, NUM_LEDS3, startIndex, 5); 
    // review2: 用了全域變數startIndex，應該用*hue
    (*hue) += speed;
    startIndex += 5;
}


// Q6 RGB white light random
void randomFlash(CRGB *leds, int numLeds, CRGB color, uint8_t chance)
{
    if (random(0, 255) < chance) //review3: 可簡化為 random8()
    {
        int randomIndex = random(0, numLeds);
        leds[randomIndex] = color;
    }
    fadeToBlackBy(leds, numLeds, 100);
}

// Q7
void breathEffect(CRGB *leds2, int numLeds, CRGB color, uint8_t speed)
{
    currentBrightness = beatsin8(speed, 0, 255);
    // review4: currentBrightness最好不要用全域變數，以免有其他地方修改它，造成混亂
    // review5: 沒有用過currentBrightness
    fill_solid(leds2, numLeds, color);
    // review5: 加入 nscale8(leds, numLeds, brightness);
}

// Q9
void waveEFfects(CRGB *leds, uint8_t *colorIndex, int NUM_LEDS)
{
    for (int i = 0; i < NUM_RGB1; i++) 
    // review1: 用區域變數NUM_LEDS，避免用全域變數NUM_RGB1
    {
        uint8_t colorIndex = random8(0, 255);
        leds_RGB1[i] = ColorFromPalette(heatmapPalette, colorIndex, 255, LINEARBLEND);
        // review1: 用區域變數leds，避免用全域變數leds_RGB1
    }
    fadeToBlackBy(leds_RGB1, NUM_RGB1, 10);
}

void loop()
{
    // CRGB color = CRGB::White;
    // randomFlash(NUM_LEDS, color, chance);

    // Q1 RGB white light
    // fill_solid(leds_RGB1, NUM_RGB1, CRGB(255,255,255));

    // Q2 RGB stay rainbow
    // fill_rainbow(leds_RGB1, NUM_RGB1, startIndex, 5);
    // startIndex += 5;

    // RGB white light random
    // randomIndex = random(0, NUM_RGB1);
    // leds_RGB1[randomIndex] = CRGB::White;
    // leds_RGB1[randomIndex] = color
    // fadeToBlackBy(leds_RGB1, NUM_RGB1, 100);

    // Q5
    // dynamicRainbow(leds_RGB3, NUM_RGB3, &hue, speed);

    // Q6
    // CRGB color = CRGB::White;
    // randomFlash(leds_RGB1, NUM_RGB1, color, chance);

    // Q7
    // CRGB color1 = CHSV(0, 255, currentBrightness);
    // breathEffect(leds_RGB2, NUM_RGB2, color1, speed);

    // Q9
    // fill_palette(leds_RGB1, NUM_RGB1, 0, 255 / NUM_RGB1, heatmapPalette, 255, LINEARBLEND);
    // leds_RGB1[5] = ColorFromPalette(heatmapPalette, 10, 255, LINEARBLEND);
    EVERY_N_MILLISECONDS(10)
    {
        waveEFfects(leds_RGB1, &colorIndex, NUM_LEDS);
    }

    FastLED.show();
    // delay(10);
    // Q4

    // Q3
    // randomIndex = random(0, 6);
    // digitalWrite(LED_PIN[randomIndex], HIGH);
    // delay(100);
    // digitalWrite(LED_PIN[randomIndex], LOW);
    // delay(50);
}
