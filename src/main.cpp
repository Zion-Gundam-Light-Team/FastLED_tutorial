#include <Arduino.h>

#define LED_1_NUM 100

bool isLedon = false;
char brightness1 = 0;
int brightness2 = 0;
unsigned long latestTime = 0;

int PINs[5] = {9, 10, 11, 12, 13};
void setup()
{
    Serial.begin(115200);
    for (int i = 0; i <= 4; i++)
    {
        pinMode(PINs[i], OUTPUT);
    }
}
void loop()
{
    for (int i = 0; i <= 100; i++)
    {
        for (int j = 0; j <= 4; j++) // -> i= i+1;
        {
            analogWrite(PINs[j], i);
        }
        delay(10);
    }

    for (int i = 0; i <= 100; i++)
    {
        for (int j = 0; j <= 4; j++)
        {
            analogWrite(PINs[j], 100 - i); // 0 -> 1->...->100
            delay(10);
        }
    }

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
}

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