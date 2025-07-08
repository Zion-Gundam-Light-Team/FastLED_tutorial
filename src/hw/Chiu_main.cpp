#include <Arduino.h>
#include <FastLED.h>

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define PIN_RGB1 1
#define NUM_RGB1 10

int allLed [6] = {1, 2, 3, 4, 5, 6};
int ledCount = sizeof(allLed) / sizeof(allLed[0]);

uint8_t randomIndex = 0;
static uint8_t startIndex = 0;
CRGB leds_RGB1[NUM_RGB1];

void setup()
{
    Serial.begin(115200);
    for (int pin = 1; pin <= 6; pin++) {
        pinMode(pin, OUTPUT);
    }

    FastLED.addLeds<LED_TYPE, PIN_RGB1, COLOR_ORDER>(leds_RGB1,NUM_RGB1);
}

int dynamicRainbow(CRGB *leds, int numLeds, uint8_t *hue, uint8_t speed){
    // review1: 如果沒有要返回值，用void代替
    fill_rainbow(leds, numLeds, *hue, speed);
    startIndex += 5;
    // review2: startIndex在這裡沒有意義
    // 應該是(*hue) += 5;
}

int randomFlash(CRGB *leds, int numLeds, CRGB color, uint8_t chance){
    if(random8()<=chance){
        randomIndex = random8(1, numLeds); // review3: 應該是random8(0, numLeds)
        leds_RGB1[randomIndex] = color;
        fadeToBlackBy(leds, numLeds, 100); // review4: 要放在if外面
    }
}

int breathEffect(CRGB *leds, int numLeds, CRGB color, uint8_t speed){
    fill_solid(leds, numLeds, color);
    uint8_t breath = beatsin8(speed, 0, 255);
    nscale8(leds, numLeds, breath);
}

void loop()
{
    // answer 1
    // for(int i : allLed){
    //     digitalWrite(i, HIGH);
    // }

    // answer 2
    // for(int i : allLed){
    //     digitalWrite(i, HIGH);
    //     delay(100);
    // }

    // answer 3
    // for(int i = sizeof(allLed); i<=ledCount; i--){
    //     digitalWrite(i, LOW);
    //     delay(100);
    // }

    // answer 4
    // for(int i = 0; i<=ledCount; i++){
    //     digitalWrite(i, HIGH);
    //     delay(100);
    // }
    // for(int i = ledCount; i>=1; i--){
    //     digitalWrite(i, LOW);
    //     delay(100);
    // }

    //lesson 2

    // RGB全條白色長著
    // fill_solid(leds_RGB1, NUM_RGB1, CRGB::White);
    // RGB靜態彩虹
    // fill_rainbow(leds_RGB1, NUM_RGB1, 0, 5);
    // 1-6號單色燈：隨機亂閃(50%)
    // randomFlash(leds_RGB1, NUM_RGB1, CRGB::White, 128);
    // RGB動態彩虹
    // dynamicRainbow(leds_RGB1, NUM_RGB1, &startIndex, 5);
    // 呼吸燈
    // breathEffect(leds_RGB1, NUM_RGB1, CRGB::White, 20);
    // FastLED.show();
    // delay(10);
    // startIndex +=5;
}




