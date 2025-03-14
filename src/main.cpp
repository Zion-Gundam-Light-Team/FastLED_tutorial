#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN_RGB1 4
#define LED_PIN_5 5
#define LED_PIN_6 6

#define NUM_LEDS_RGB1 64

CRGB leds_RGB1[NUM_LEDS_RGB1];

void setup()
{
    Serial.begin(115200);
    FastLED.addLeds<WS2812B, LED_PIN_RGB1, GRB>(leds_RGB1, NUM_LEDS_RGB1);
    FastLED.clear();
    FastLED.setBrightness(40);
    pinMode(LED_PIN_5, OUTPUT);
    pinMode(LED_PIN_6, OUTPUT);
}
void loop()
{
}
