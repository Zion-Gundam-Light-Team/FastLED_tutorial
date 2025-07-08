#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN1 1
#define LED_PIN2 2
#define LED_PIN3 3
#define LED_PIN4 4
#define LED_PIN5 5
#define LED_PIN6 6
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define PIN_RGB1 16
#define PIN_RGB2 13
#define PIN_RGB3 15
#define NUM_RGB1 10
#define NUM_RGB2 10
#define NUM_RGB3 10
CRGB leds_RGB1[NUM_RGB1];
CRGB leds_RGB2[NUM_RGB2];
CRGB leds_RGB3[NUM_RGB3];
uint8_t startIndex = 0;
int pinArray1[6] = {LED_PIN1, LED_PIN2, LED_PIN3, LED_PIN4, LED_PIN5, LED_PIN6};
uint8_t brightness = 100;
// int a;
// int *leds;
// leds = &a;
// a = 5;
// uint8_t speed = 5;
// CRGBPalette16 waveMapPalette = waveMap;
uint8_t PaletteIndex = 0;

void setup()
{
    Serial.begin(115200);
    FastLED.addLeds<LED_TYPE, PIN_RGB1, COLOR_ORDER>(leds_RGB1, NUM_RGB1);
    FastLED.addLeds<LED_TYPE, PIN_RGB2, COLOR_ORDER>(leds_RGB2, NUM_RGB2);
    FastLED.addLeds<LED_TYPE, PIN_RGB3, COLOR_ORDER>(leds_RGB3, NUM_RGB3);
    pinMode(LED_PIN1, OUTPUT);
    pinMode(LED_PIN2, OUTPUT);
    pinMode(LED_PIN3, OUTPUT);
    pinMode(LED_PIN4, OUTPUT);
    pinMode(LED_PIN5, OUTPUT);
    pinMode(LED_PIN6, OUTPUT);
}

void dynamicRainbow(CRGB *leds, int numLeds, uint8_t *hue, uint8_t speed)
{
    fill_rainbow(leds, numLeds, *hue, 5); //    review3: hueDelta 代替hardcode的5
    (*hue) += speed;
}

//  void dynamicRainbow(CRGB * leds, int numLeds, uint8_t *hue, uint8_t speed, uint8_t deltahue)
//  {
//      fill_rainbow(leds, numLeds, *hue, deltahue);
//      (*hue) += speed;
//  }



// uint8_t randomIndex = ramdom8(0, NUM_RGB2)
// leds_RGB2[randomIndex] = CRGB::White;
// fadeToBlackBy(leds_RGB2, NUM_RGB2, 100);
void randomFlash(CRGB *leds, int numLeds, CRGB color, uint8_t chance)
{
    if (random(0, 256) <= chance)
    // review2: 可以用 random8 代替
    // 條件永遠成立（255 <= 255）
    // chance可直接忽略
    {
        uint8_t randomIndex = random(0, numLeds);
        leds[randomIndex] = color;
        fadeToBlackBy(leds, numLeds, 100);
    }
}

// uint8_t brightnessRGB = beatsin8(30, 0, 255);
// fill_solid(leds_RGB3, NUM_RGB3, CHSV(0,255,brightnessRGB));
void breathEffect(CRGB *leds, int numLeds, uint8_t color, uint8_t speed)
{
    uint8_t brightnessRGB = beatsin8(speed, 0, 255);
    fill_solid(leds, numLeds, CHSV(color, 255, brightnessRGB));
}

// https://www.selecolor.com/en/hsv-color-picker/
// https://www.selecolor.com/en/rgb-color-picker/
DEFINE_GRADIENT_PALETTE(waveMap){
    0, 0, 255, 64,
    128, 0, 234, 255,
    200, 0, 106, 255,
    255, 255, 255, 255};

void waveEffect(CRGB *leds, int numLeds, CRGBPalette16 color)
{
    CRGBPalette16 waveMapPalette = color;
    fill_palette(leds, numLeds, 0, 255 / numLeds, color, 255, LINEARBLEND);
    for (int i = 0; i < 4; i++)
    {
        leds[i] = ColorFromPalette(color, PaletteIndex, 255, LINEARBLEND);
        fadeToBlackBy(leds, numLeds, 10);
        PaletteIndex += 50;
    }

    for (int i = 4; i < 8; i++)
    {
        leds[i] = ColorFromPalette(color, PaletteIndex, 255, LINEARBLEND);
        fadeToBlackBy(leds, numLeds, 10);
        PaletteIndex += 50;
    }

    for (int i = 8; i < 12; i++)
    {
        leds[i] = ColorFromPalette(color, PaletteIndex, 255, LINEARBLEND);
        fadeToBlackBy(leds, numLeds, 10);
        PaletteIndex += 50;
    }
}

void loop()
{

    //-----------------基本題目----------------

    // 1️ RGB全條白色長著
    fill_solid(leds_RGB1, NUM_RGB1, CRGB::White);

    // 2️ RGB靜態彩虹
    fill_rainbow(leds_RGB1, NUM_RGB1, startIndex, 5);

    // 3️ 1-6號單色燈：隨機亂閃
    analogWrite(pinArray1[6] = random8(0, 7), brightness);
    analogWrite(pinArray1[6] = random8(0, 7), 0); //review2： 只控制一個燈的亮度（0-7 brightness)

    // review2:
    // EVERY_N_MILLISECONDS(100)
    // {
    //     for (int i = 0; i < 6; i++)
    //     {
    //         analogWrite(pinArray1[i], 0);
    //     }
    //     int randomPin = random(0, 6);
    //     analogWrite(pinArray1[randomPin], brightness);
    // }


    // 4️ RGB動態彩虹
     fill_rainbow(leds_RGB1, NUM_RGB1, startIndex, 5);
     delay(500);
     startIndex += 5;

    //----------------中等題目-----------------

    // 5️ 承第4題，改用function寫: dynamicRainbow(CRGB *leds, int numLeds, uint8_t *hue, uint8_t speed)
    //(提示: hue為初始顏色，speed為變化速度)
     EVERY_N_MILLISECONDS(500){
         dynamicRainbow(leds_RGB1, NUM_RGB1, &startIndex, 5);
     } // reivew" 用pointer代替hardcode


     // 6 RGB：隨機亂閃function: randomFlash(CRGB *leds, int numLeds, CRGB color, uint8_t chance)
     //(提示: chance 範圍是0-255，255=100%機率)
     EVERY_N_MILLISECONDS(100){
         randomFlash(leds_RGB2, NUM_RGB2, CRGB::White, 255);
     }


    // 7️ RGB：呼吸燈function: breathEffect(CRGB *leds, int numLeds, CRGB color, uint8_t speed)
    //(提示: 使用 beatsin8)
     EVERY_N_MILLISECONDS(50){
         breathEffect(leds_RGB3, NUM_RGB3, 0, 30);
     }

    // ️8 三條RGB燈帶同時運行:
    // 1. 燈帶1: 白色隨機閃爍
    EVERY_N_MILLISECONDS(100)
    {
        randomFlash(leds_RGB2, NUM_RGB2, CRGB::White, 255);
    }

    // 2. 燈帶2: 紅色呼吸效果
     EVERY_N_MILLISECONDS(50){
         breathEffect(leds_RGB3, NUM_RGB3, 0, 30);
     }

    // 3. 燈帶3: 動態彩虹流動
    EVERY_N_MILLISECONDS(500)
    {
        dynamicRainbow(leds_RGB1, NUM_RGB1, &startIndex, 5);
    }

    //----------------挑戰題目-----------------

    // 9️ 水光燈效果: waveEffect(CRGB *leds, int numLeds, CRGB color)
    //(提示: 使用 ColorFromPalette , fadeToBlackBy, EVERY_N_MILLISECONDS)

    EVERY_N_MILLISECONDS(100)
    {
        waveEffect(leds_RGB3, NUM_RGB3, waveMap);

        //---Start v2---
        // CRGBPalette16 waveMapPalette = waveMap;
        // fill_palette(leds_RGB3, NUM_RGB3, 0, 255 / NUM_RGB3, waveMapPalette, 255, LINEARBLEND);
        // for (int i = 0; i < 4; i++)
        // {
        //     leds_RGB3[i] = ColorFromPalette(waveMapPalette, PaletteIndex, 255, LINEARBLEND);
        //     fadeToBlackBy(leds_RGB3, NUM_RGB3, 10);
        //     PaletteIndex += 50;
        // }

        // for (int i = 4; i < 8; i++)
        // {
        //     leds_RGB3[i] = ColorFromPalette(waveMapPalette, PaletteIndex, 255, LINEARBLEND);
        //     fadeToBlackBy(leds_RGB3, NUM_RGB3, 10);
        //     PaletteIndex += 50;
        // }

        // for (int i = 8; i < 12; i++)
        // {
        //     leds_RGB3[i] = ColorFromPalette(waveMapPalette, PaletteIndex, 255, LINEARBLEND);
        //     fadeToBlackBy(leds_RGB3, NUM_RGB3, 10);
        //     PaletteIndex += 50;
        // }
        //---End v2---

        //---Start v1---
        // CRGBPalette16 waveMapPalette = waveMap;
        // leds_RGB3[0] = ColorFromPalette(waveMapPalette, PaletteIndex, 255, LINEARBLEND);
        // fadeToBlackBy(leds_RGB3, NUM_RGB3, 100);
        // PaletteIndex+=50;
        //---END v1---
    }
    FastLED.show();
}

// review4: EVERY_N_MILLISECONDS是static, 有機會造成燈條之間的不同步，
// 建議用millis()