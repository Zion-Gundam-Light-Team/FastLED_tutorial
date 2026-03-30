#include <Arduino.h>
#include <FastLED.h>

uint8_t randomChance2 = 80;
int interval2 = 100;
int brightnessLow2 = 0;
int brightnessHigh2 = 100;
unsigned long onDuration2 = 10;

void setup() {
    Serial.begin(115200);

    for (int i = 9; i <= 13; i++) {
        pinMode(i, OUTPUT);
    }
}

void led_random_flash(int pin, int interval, uint8_t randomChance,
                      int brightnessLow, int brightnessHigh,
                      unsigned long onDuration) {
    if (random8() < randomChance) {  // 40/255 = 15%
        analogWrite(pin, brightnessHigh);
        delay(onDuration);  // Flash for 10ms
        analogWrite(pin, brightnessLow);
    }
    delay(interval);
}

void loop() {
    for (int i = 9; i <= 13; i++) {
        if (random8() < 40) {  // random8() choose 0-39,
            analogWrite(i, 100);
            delay(10);  // Flash for 10ms
            analogWrite(i, 0);
        }
    }
    delay(100);  // Interval = 100ms
    // for (int i = 9; i <= 13; i++) {
    //     led_random_flash(i, interval2, randomChance2, brightnessLow2,
    //                      brightnessHigh2, onDuration2);
    // }
}