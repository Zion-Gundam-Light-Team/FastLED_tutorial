#include <Arduino.h>
#include <FastLED.h>

int pins[5] = {9, 10, 11, 12, 13};

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define PIN_RGB1 1
#define NUM_RGB1 100
#define PIN_RGB2 2
#define NUM_RGB2 10
CRGB leds_RGB1[NUM_RGB1];
CRGB leds_RGB2[NUM_RGB2];

uint8_t randomIndex = 0;
uint8_t startIndex = 0;

// Q5 variables
// unsigned long onDuration = 10;
// uint8_t deltaHue = 15;
// uint8_t speed = 5;

// Q5 functions
void dynamicRainbow(CRGB* leds, int numLeds, uint8_t* hue,
                    uint8_t speed) {  // Pointer =array
    fill_rainbow(leds, numLeds, *hue,
                 speed);  // array [0] = 1000, array[1] = 1001
    FastLED.show();
    delay(100);
    *hue += speed;
}

// Q6 variables
// uint8_t randomChance2 = 150;
// int interval2 = 100;
// int brightnessLow2 = 0;
// int brightnessHigh2 = 100;
// unsigned long onDuration2 = 10;

// Q6 functions
// void led_random_flash(int pin, int interval, uint8_t randomChance,
//                       int brightnessLow, int brightnessHigh,
//                       unsigned long onDuration) {
//     if (random8() < randomChance) {  // 40/255 = 15%
//         analogWrite(pin, brightnessHigh);
//         delay(onDuration);  // Flash for 10ms
//         analogWrite(pin, brightnessLow);
//     }
//     delay(interval);
// }

// Q7 variables
//  int brightnessLow3 = 0;
//  int brightnessHigh3 = 255;
//  uint8_t frequency3 = 20;
//  unsigned long onDuration3 = 100;

// Q7 functions
// void breathEffect(CRGB *leds, int numLeds, uint8_t frequency, int
// brightnessLow, int brightnessHigh, unsigned long onDuration, uint8_t hue){
//     uint8_t currentBrightness = beatsin8(frequency, brightnessLow,
//     brightnessHigh); // 60/ frequency(20) = 3s fill_solid(leds, numLeds,
//     CHSV(hue, 255, currentBrightness)); FastLED.show(); delay(onDuration);
// }

// Q8 variables
DEFINE_GRADIENT_PALETTE(ocean_gp){0,   0,   255, 245,   // #00ffF5
                                  46,  0,   21,  255,   // #0015ff
                                  179, 12,  250, 0,     // #0cfa00
                                  230, 0,   255, 245,   // #00ffF5
                                  255, 250, 255, 245};  // #FAFFF5

CRGBPalette16 oceanPalette = ocean_gp;

uint8_t colorIndex = 0;

// Q8 functions
// 參數：
// - leds：CRGB燈帶object
// - colorIndex：用於從colorPalette中獲取顏色value
// - NUM_LEDS：燈珠數目

// 1. Modify the values
// 2. Array is a pointer
void waveEFfects(CRGB* leds, uint8_t* colorIndex, int NUM_LEDS) {
    fill_palette(leds, NUM_LEDS, *colorIndex, 5, oceanPalette, 255,
                 LINEARBLEND);
    FastLED.show();
    delay(50);
    *colorIndex += 5;
}

void setup() {
    Serial.begin(115200);           // i++
    for (int i = 0; i <= 4; i++) {  // -> i+1
        pinMode(pins[i], OUTPUT);
    }
    FastLED.addLeds<LED_TYPE, PIN_RGB1, COLOR_ORDER>(leds_RGB1, NUM_RGB1);
    FastLED.addLeds<LED_TYPE, PIN_RGB2, COLOR_ORDER>(leds_RGB2, NUM_RGB2);
    FastLED.setBrightness(50);
}

// HW2
void loop() {
    // Q1
    // fill_solid(leds_RGB1, NUM_RGB1, CRGB(100, 100, 100));

    // Q2
    // fill_rainbow(leds_RGB1, NUM_RGB1, startIndex, 15);

    // Q3
    // {
    // for (int i = 9; i <= 13; i++) {
    //     if (random8(0,255) < 40) {  // 40/255 = 15%
    //         analogWrite(i, 100);
    //         delay(10);  // Flash for 10ms
    //         analogWrite(i, 0);
    //     }
    // }
    // delay(100);  // Interval = 100ms
    //     }
    // }

    // Q4
    // fill_rainbow(leds_RGB1, 10, 0, 15);
    // FastLED.show();
    // delay(10);
    // startIndex += 5;

    // Q5
    // dynamicRainbow(leds_RGB1, NUM_RGB1, &startIndex, speed);

    // Q6
    //     for (int i = 9; i <= 13; i++) {
    //         if (random8() < randomChance2) {  // 40/255 = 15%
    //             analogWrite(i, brightnessHigh2);
    //             delay(onDuration2);  // Flash for 10ms
    //             analogWrite(i, brightnessLow2);
    //         }
    //     }
    //     delay(interval2);  // Interval = 100ms
    //     for (int i = 9; i <= 13; i++) {
    //         led_random_flash(i, interval2, randomChance2, brightnessLow2,
    //                          brightnessHigh2, onDuration2);
    // }

    // Q7
    // breathEffect(leds_RGB1, NUM_RGB1, frequency3, brightnessLow3,
    // brightnessHigh3, onDuration3, 0); breathEffect(leds_RGB2, NUM_RGB2, 15,
    // brightnessLow3, brightnessHigh3, onDuration3, 85);

    // Q8
    waveEFfects(leds_RGB1, &colorIndex, NUM_RGB1);
}

// HW1
/*
void loop()
{
    for(int j = 0; j <= 100; j++){
        for(int i = 0; i <= 4; i++){ // -> i+1
            analogWrite(pins[i], j); // 0->1->2->3->...->100
        }
        delay(10);
    }
    for(int j = 0; j <= 100; j++){
        for(int i = 0; i <= 4; i++){
            //if i--,then pins[0] -> pins[-1] has error
            // if i++,then pins[0](9) -> pins[1](10)
            analogWrite(pins[i], 100-j); // 100->99 ->98->...->0
        }
        delay(10);
    }
}
*/
