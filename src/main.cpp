#include <Arduino.h>
#include <FastLED.h>

#define LED_type WS2812B
#define color_order GRB
#define pin_rgb1 1
#define num_rgb1 10
#define LED_1_PIN_1
#define NUM_LEDS 60
#define DATA_PIN 1

DEFINE_GRADIENT_PALETTE(blue_wave_gp){
    0, 0, 0, 20,        // 深海藍
    64, 0, 50, 150,     // 中藍
    128, 30, 144, 255,  // 海藍
    192, 100, 200, 255, // 淺藍
    255, 200, 240, 255  // 波浪白
};

DEFINE_GRADIENT_PALETTE(rainbow_gp){
    0, 255, 0, 0,     // Red
    42, 255, 255, 0,  // Yellow
    85, 0, 255, 0,    // Green
    127, 0, 255, 255, // Cyan
    170, 0, 0, 255,   // Blue
    212, 255, 0, 255, // Magenta
    255, 255, 0, 0    // Back to Red
};
CRGBPalette16 rainbow_p = rainbow_gp;

CRGBPalette16 blue_wave_p = blue_wave_gp;

CRGB leds_rgb1[num_rgb1];
uint8_t startindex2 = 0;
uint8_t currentBrightness = 0;
uint8_t Randomindex = 0;
uint8_t i = 0;
uint8_t colorIndex[NUM_LEDS];
int PINs[] = {9, 10, 11, 12, 13};

// Pointers: Modify the values in function
// startIndex-> address
// *startIndex -> value
// void waveEFfects(CRGB *leds, uint8_t *colorIndex, int numLEDs)
// {
//     blue_wave_p
//     uint8_t brightness = beatsin8(speed, 0, 255);
//     CRGB breatheColor = color;
//     breatheColor.nscale8(brightness);

//     fill_solid(leds, numLeds, breatheColor);

// for (int i = 0; i < numLeds; i++)
// {
//     if (random(0, 100) < chance)
//     {
//         CRGB randomColor = CRGB(random(0, 25), random(0, 50), random(0, 75));
//         // logic of random color
//         // randomColor =
//         leds[i] = randomColor;
//     }
//     else
//     {
//         leds[i] = CRGB::Black;
//     }
// }
// chance = random(0, chance);

// to do random flash

// fill_rainbow(leds, numLeds, *startindex, 5);(5)
// FastLED.show();
// delay(500);
// *startindex += speed; // 0->5->10->15 ...
// }

void gradientDynamicPalette(CRGB *leds, int numLeds, uint8_t *startIndex, CRGBPalette16 palette, uint8_t speed)
{
    fill_palette(leds, numLeds, *startIndex, 15, palette, 255, LINEARBLEND);
    FastLED.show();
    delay(random8(50, 150));
    *startIndex += speed;
}

void setup()
{
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds_rgb1, NUM_LEDS);
    FastLED.setBrightness(180);
    for (int i = 0; i < NUM_LEDS; i++)
    {
        colorIndex[i] = i * 5;
    }
    // currentPalette = OceanColors_p;

    // Serial.begin(115200);(6  )
    // FastLED.addLeds<LED_type, pin_rgb1, color_order>(leds_rgb1, num_rgb1);
    // FastLED.setBrightness(100);
    // randomFlash(leds_rgb1, num_rgb1, CRGB::White, 100);

    // Serial.begin(115200);(5)
    // FastLED.addLeds<LED_type, pin_rgb1, color_order>(leds_rgb1, num_rgb1);
    // FastLED.setBrightness(100);
    // fill_solid(leds_rgb1, num_rgb1, CRGB::Black);
}

void loop()
{
    gradientDynamicPalette(leds_rgb1, num_rgb1, &startindex2, rainbow_p, 15);
    // fill_rainbow(leds_rgb1, num_rgb1, startindex2, 5);
    // FastLED.show();
    // delay(500);
    // startindex2 += 5; // 0->5->10->15 ...

    // waveEffects(leds_rgb1, colorIndex, NUM_LEDS);
    // fill_rainbow(leds_rgb1, num_rgb1, startindex2, 5);
    // fill_palette(leds_rgb1, num_rgb1, startindex2, 15, blue_wave_p, 255, LINEARBLEND);
    // FastLED.show();
    // delay(100);
    // startindex2 += 5;

    // breathEffect(leds_rgb1, NUM_LEDS, CRGB::Red, 12);(7)

    // FastLED.show();
    // delay(10);
}

// & -> address
// dynamicRainbow(leds_rgb1, num_rgb1, &startindex2, 5);
// (5)
// Randomindex = random(0, 4);
// analogWrite(PINs[Randomindex], 1);
// delay(500);

// delay(2000);
// for (int i = 0; i <= 4; i++) // -> i= i+1;
// {
//     analogWrite(PINs[i], 0);
//     delay(500);
// Randomindex = random(0, num_rgb1);
// // fadeToBlackBy(leds_rgb1, num_rgb1, 10);

// fill_rainbow(leds_rgb1, num_rgb1, i, 20);
// FastLED.show();
// i += 20;
// delay(100);

// {   currentBrightness =beatsin8(10,0,255);
//     fill_solid(leds_rgb1, num_rgb1,CHSV(0,255,currentBrightness));
//     FastLED.show();

//     fadeToBlackBy(leds_rgb1, num_rgb1, 10);

// fill_solid(leds_rgb1, num_rgb1, CRGB::Red);
//  EVERY_N_MILLISECONDS(500) {
//  fill_rainbow(leds_rgb1, num_rgb1, startindex, 5);
//  FastLED.show();
//  delay(500);
//  startindex += 5;
//  }
//  fill_palette(leds_rgb1, num_rgb1, startindex, 255 /num_rgb1, heatmap_palette, 255, LINEARBLEND);
//  FastLED.show();
//  delay(500);
//  FastLED.setBrightness(255);

bool isLedon = false;
char brightness1 = 0;
int brightness2 = 0;
unsigned long latestTime = 0;

//

// for (int i = 0; i <= 4; i++) // -> i= i+1;
// {
//     analogWrite(PINs[i], 1);
//     delay(500);
// }
// delay(2000);
// for (int i = 0; i <= 4; i++) // -> i= i+1;
// {
//     analogWrite(PINs[i], 0);
//     delay(500);
// }

// void setup()
// {
// }
// void loop()
// {
//     if (digitalRead(2) == HIGH)
//     {
//         if (brightness1 < 255)
//     }
// }