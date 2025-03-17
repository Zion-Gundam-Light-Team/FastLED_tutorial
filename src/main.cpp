#include <Arduino.h>

#define LED_PIN_4 4
#define LED_PIN_5 5
#define LED_PIN_6 6

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN_4, OUTPUT);
    pinMode(LED_PIN_5, OUTPUT);
    pinMode(LED_PIN_6, OUTPUT);
}
void loop()
{
    Serial.println("hello world");
    delay(1000);
}
