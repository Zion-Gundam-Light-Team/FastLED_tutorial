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

#define NUM_LEDS 6 // Number of LEDs
const int LED_PINS[NUM_LEDS] = {1,2,3,4,5,6};

#define framePerSecond 50 

int brightness = 0;

CRGB leds_RGB1[NUM_RGB1];
CRGB leds_RGB2[NUM_RGB2];
CRGB leds_RGB3[NUM_RGB3];

uint8_t startIndex = 0;
uint8_t randomIndex = 0;


//-------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  // <strip型號, GPIO腳位, RGB>(CRGB array name, 燈珠no.)
  FastLED.addLeds<LED_TYPE, PIN_RGB1, COLOR_ORDER>(leds_RGB1, NUM_RGB1);
  FastLED.addLeds<LED_TYPE, PIN_RGB2, COLOR_ORDER>(leds_RGB2, NUM_RGB2);
  FastLED.addLeds<LED_TYPE, PIN_RGB3, COLOR_ORDER>(leds_RGB3, NUM_RGB3);
}

void loop() {

  //Q1 RGB全條白色長著
    // fill_solid(leds_RGB1, NUM_RGB1, CRGB(255,255,255));
    // FastLED.show();

  // //Q2 RGB靜態彩虹
  //   fill_rainbow(leds_RGB1, NUM_RGB1, startIndex, 20);
  //   fill_rainbow(leds_RGB2, NUM_RGB2, startIndex, 20);
  //   fill_rainbow(leds_RGB3, NUM_RGB3, startIndex, 20);
  //   FastLED.show();

  //Q3 1-6號單色燈：隨機亂閃

// randomIndex = random(0, NUM_RGB3); 
// leds_RGB3[randomIndex] = CRGB::White; 
// fadeToBlackBy(leds_RGB3, NUM_RGB3, 40); 
// FastLED.show(); 
// delay(100);


  // //Q4 RGB動態彩虹
   EVERY_N_MILLISECONDS(15){
    fill_rainbow(leds_RGB2, NUM_RGB2, startIndex, 20);
    FastLED.show();
    startIndex += 10;
  }

}


// void dynamicRainbow(CRGB *leds, int numLeds, uint8_t *hue, uint8_t speed) {
//   CRGB color = CHSV(*hue, 255, 100);
//   fill_solid(leds, numLeds, color);
//   (*hue) += speed;
// }

//-------------------------------------------------------------------
// int LED[6] = {5,6,7,8,9,10};

// DEFINE_GRADIENT_PALETTE(wave_gp) {
   
 
// };

// CRGBPalette16 waterPalette = wave_gp;
// bool isOn[6] = {false, false, false, false, false, false};
// unsigned long lastUpdate[6] = {0, 0, 0, 0, 0, 0};
// unsigned long effectStartTime = 0;
// uint8_t currentEffect = 0;

// void setup() {
//   Serial.begin(115200);
//   FastLED.addLeds<LED_TYPE, PIN_RGB1, COLOR_ORDER>(leds_RGB1, NUM_RGB1);
//   FastLED.addLeds<LED_TYPE, PIN_RGB2, COLOR_ORDER>(leds_RGB2, NUM_RGB2);
//   FastLED.addLeds<LED_TYPE, PIN_RGB3, COLOR_ORDER>(leds_RGB3, NUM_RGB3);
//   for(int i = 5; i<11; i++){
//     pinMode(i,OUTPUT);
//   }
//   effectStartTime = millis();
// }

// void loop() {
//   if (millis() - effectStartTime >= 10000) {
//     effectStartTime = millis();
//     currentEffect = (currentEffect + 1) % 8;
//     FastLED.clear();
//   }

//   switch(currentEffect) {
//     case 0:
      
//       break;
//     case 1: 
//       break;
//     case 2:
//       break;
//     case 3:
//       break;
//     case 4:
//       break;
//     case 5:
//       break;
//     case 6:
//       break;
//     case 7:
//       break;
//   }

//   delay(1000/framePerSecond);
//   FastLED.show();
// }

// review1: 全域只要一個FastLED.show();就可以
// review2: EVERY_N_MILLISECONDS是static, 有機會造成燈條之間的不同步，
// 建議用millis()