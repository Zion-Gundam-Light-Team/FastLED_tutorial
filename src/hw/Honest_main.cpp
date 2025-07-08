#include <Arduino.h>
#include <FastLED.h>

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define PIN_RGB1 1
#define NUM_RGB1 10
#define PIN_RGB2 2
#define NUM_RGB2 10
#define PIN_RGB3 3
#define NUM_RGB3 10

#define framePerSecond 50

int brightness = 0;

CRGB leds_RGB1[NUM_RGB1];
CRGB leds_RGB2[NUM_RGB2];
CRGB leds_RGB3[NUM_RGB3];

int LED[6] = {5, 6, 7, 8, 9, 10};

DEFINE_GRADIENT_PALETTE(wave_gp){

    0, 0, 0, 255,
    63, 0, 128, 255,
    127, 0, 255, 128,
    191, 128, 255, 0,
    255, 255, 255, 255};

CRGBPalette16 waterPalette = wave_gp;
bool isOn[6] = {false, false, false, false, false, false};
// review3: 簡化的寫法 bool isOn[6] = {false};
unsigned long lastUpdate[6] = {0, 0, 0, 0, 0, 0};
unsigned long effectStartTime = 0;
uint8_t currentEffect = 0;

void setup()
{
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE, PIN_RGB1, COLOR_ORDER>(leds_RGB1, NUM_RGB1);
  FastLED.addLeds<LED_TYPE, PIN_RGB2, COLOR_ORDER>(leds_RGB2, NUM_RGB2);
  FastLED.addLeds<LED_TYPE, PIN_RGB3, COLOR_ORDER>(leds_RGB3, NUM_RGB3);
  for (int i = 5; i < 11; i++)
  {
    pinMode(i, OUTPUT);
  }
  effectStartTime = millis();
}

void dynamicRainbow_Q4(CRGB *leds, int numLeds, uint8_t *hue, uint8_t deltahue)
{
  CRGB color = CHSV(*hue, 255, 100);
  fill_solid(leds_RGB1, NUM_RGB1, color);
  (*hue) += deltahue;
}

void dynamicRainbow(CRGB *leds, int numLeds, uint8_t *hue, uint8_t speed)
{
  CRGB color = CHSV(*hue, 255, 100);
  fill_solid(leds, numLeds, color);
  (*hue) += speed;
}

void gradientDynamicRainbow(CRGB *leds, int NUM_LEDS, uint8_t *hue, uint8_t deltahue)
{
  fill_rainbow(leds, NUM_LEDS, *hue, deltahue);
  (*hue) += 1;
}

void randomFlash(CRGB *leds, int numLeds, CRGB color, uint8_t chance)
{
  for (int i = 0; i < numLeds; i++)
  {
    if (random8() < chance)
    {
      leds[i] = color;
    }
    else
    {
      leds[i] = CRGB::Black;
    }
  }
}

void breathEffect(CRGB *leds, int numLeds, CRGB color, uint8_t speed)
{
  uint8_t brightness = beatsin8(speed, 64, 255);
  fill_solid(leds, numLeds, color);
  nscale8(leds, numLeds, brightness);
}

void waveEffect(CRGB *leds, int numLeds)
{
  EVERY_N_MILLISECONDS(30)
  {
    fadeToBlackBy(leds, numLeds, 15);

    static uint8_t wavePos = 0;
    static uint8_t waveHue = 0;

    uint8_t waveWidth = beatsin8(10, 5, 30);
    uint8_t waveSpeed = beatsin8(15, 1, 3);

    for (int i = 0; i < numLeds; i++)
    {
      uint8_t position = wavePos + (i * 12) + beatsin8(20, 0, 16);

      CRGB color = ColorFromPalette(wavePalette, position + waveHue, 255, LINEARBLEND);

      if (random8() < 3)
      {
        color = CRGB::White;
      }
      leds[i] += color;
    }

    wavePos += waveSpeed;
    waveHue += 1; // Slowly shift color over time
  }
}

void loop()
{
  if (millis() - effectStartTime >= 10000)
  {
    effectStartTime = millis();
    currentEffect = (currentEffect + 1) % 8;
    FastLED.clear();
  }

  switch (currentEffect)
  {
  case 0:
    fill_solid(leds_RGB1, NUM_RGB1, CRGB(20, 20, 20));
    break;
  case 1:
    fill_rainbow(leds_RGB2, NUM_RGB2, 0);
    break;
  case 2:
    for (int i = 5; i < 11; i++)
    {
      // review1: best practise: 直接用要loop的數目
      //  for (int i = 0; i < 6; i++) {

      // review2: lastUpdate不必用array
      //    ...
      //    if (millis() - lastUpdate[i] >= interval) {
      //    lastUpdate[i] = millis();
      //    ...
      int interval = 100;
      // review3: interval可以用常數代替, 減省記憶體
      if (millis() - lastUpdate[i - 5] >= interval)
      {
        lastUpdate[i - 5] = millis();
        if (random8() < 10)
        {
          // review4: isOn在這此沒有意義
          isOn[i - 5] = !isOn[i - 5]; // 這行可以直接移除
          brightness = random8(64, 255);
          analogWrite(i, brightness);
          delay(10);
          analogWrite(i, 0);
        }
      }
    }
    break;
  case 3:
    static uint8_t hue1 = 0;
    dynamicRainbow_Q4(leds_RGB1, NUM_RGB1, &hue1, 1);
    break;
  case 4:
    static uint8_t hue2 = 0;
    gradientDynamicRainbow(leds_RGB2, NUM_RGB2, &hue2, 10);
    break;
  case 5:
    randomFlash(leds_RGB3, NUM_RGB3, CHSV(random8(), 0, 200), 20);
    break;
  case 6:
    breathEffect(leds_RGB3, NUM_RGB3, CHSV(2, 255, 50), 1);
    break;
  case 7:
    waveEffect(leds_RGB2, NUM_RGB2);
    break;
  }

  delay(1000 / framePerSecond);
  FastLED.show();
}