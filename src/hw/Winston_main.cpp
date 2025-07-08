#include <Arduino.h>
#include <FastLED.h>

//const int LED_COUNT[] = {1,2,3,4,5,6};
const int RGB_COUNT[] = {1,2,3};

//#define LED_PIN 6
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define PIN_RGB1 4
#define PIN_RGB2 5
#define PIN_RGB3 6
#define NUM_RGB1 10
#define NUM_RGB2 10
#define NUM_RGB3 10
DEFINE_GRADIENT_PALETTE(heatmap_gp){
    0,0,0,0,
    128,255,0,0,
    200,255,255,0,
    255,255,255,255};

CRGBPalette16 heatmap_palette = heatmap_gp;
DEFINE_GRADIENT_PALETTE( waterwave_gp ) {
    0,   0,  0,255,  // Blue
    127, 0,255,  0,  // Green
    212,255,255,255, // White
    255, 0,  0,255}; // Blue (for looping)

CRGBPalette16 waterwave_palette = waterwave_gp;
CRGB leds_RGB1[NUM_RGB1];
CRGB leds_RGB2[NUM_RGB2];
CRGB leds_RGB3[NUM_RGB3];
uint8_t startIndext = 0;
uint8_t currenttBrightness = 0;
uint8_t randomIndex = 0;
uint8_t starthue = 128;
//bool isOn = true;


void setup(){
    Serial.begin(115200);
    // for (int i = 0; i < LED_PIN; i++)
    // {
    //     pinMode(LED_COUNT[i],OUTPUT);
    // }
    FastLED.addLeds<LED_TYPE, PIN_RGB1, COLOR_ORDER>(leds_RGB1, NUM_RGB1);
    FastLED.addLeds<LED_TYPE, PIN_RGB2, COLOR_ORDER>(leds_RGB2, NUM_RGB2);
    FastLED.addLeds<LED_TYPE, PIN_RGB3, COLOR_ORDER>(leds_RGB3, NUM_RGB3);
    FastLED.setBrightness(50);
}

// 5️⃣承第4題，改用function寫:
// dynamicRainbow(CRGB *leds, int numLeds, uint8_t *hue, uint8_t speed)
// (提示: hue為初始顏色，speed為變化速度)
void dynamicRainbow(CRGB *leds, int numLeds, uint8_t *hue, uint8_t speed){

    fill_rainbow(leds, numLeds, startIndext, speed);
    // review3: 使用了外部的startIndext，易造成混淆
    // review3: 封裝 encapsulation
    FastLED.show();
    delay(50);
    startIndext += 5;
}
//review3: 建議：
// void dynamicRainbow(CRGB *leds, int numLeds, uint8_t *hue, uint8_t speed)
// {
//     fill_rainbow(leds, numLeds, *hue, speed);
//     (*hue) += 5;
// }

// 6️⃣RGB：隨機亂閃function:
// randomFlash(CRGB *leds, int numLeds, CRGB color, uint8_t chance)
// (提示: chance 範圍是0-255，255=100%機率)
void randomFlash(CRGB *leds, int numLeds, CRGB color, uint8_t chance){
randomIndex = random(0,NUM_RGB1); 
    leds_RGB1[randomIndex] = CRGB::White;
    fadeToBlackBy(leds_RGB1,NUM_RGB1, 100);
    FastLED.show();
    delay(20);
}

// 7️⃣RGB：呼吸燈function:
// breathEffect(CRGB *leds, int numLeds, CRGB color, uint8_t speed)
// (提示: 使用 beatsin8)
void breathEffect(CRGB *leds, int numLeds, CRGB color, uint8_t speed){
    
    uint8_t breath = beatsin8(speed, 50,255);

    for(int i = 0 ; i < numLeds; i++){
        leds[i] = color;
        leds[i].nscale8(breath);
    }
}
// review4: 再簡化的寫法：
// void breathEffect(CRGB *leds, int numLeds, CRGB color, uint8_t speed)
// {
//     uint8_t brightness = beatsin8(speed, 50, 255);
//     fill_solid(leds, numLeds, color);
//     nscale8(leds, numLeds, brightness);
// }

// 第九題還是沒完成


// 9水光燈效果:
// waveEffect(CRGB *leds, int numLeds, CRGB color)
// (提示: 使用 ColorFromPalette , fadeToBlackBy, EVERY_N_MILLISECONDS)
void waveEffect(CRGB *leds, int numLeds, CRGBPalette16& palette){

    static uint8_t start = 0;
    static uint8_t hueoffset = 0;

    //wave cycle
    EVERY_N_MILLISECONDS(20);{
        start+=2;
        hueoffset+=1;
    }

    for (int i = 0 ;i < numLeds;i++ ){
    uint8_t position = (i * 255/numLeds);
    uint8_t colorIndex = start + position;
    leds[i] = ColorFromPalette(palette, colorIndex);
    }

    fadeToBlackBy(leds,numLeds, 20);

    for(int i = 0 ; i < numLeds; i++){
        uint8_t breath = sin8(start * 5 + i *20);
        leds[i].nscale8(breath);
    }
    FastLED.show();
}

// HW88️⃣三條RGB燈帶同時運行:
// 1. 燈帶1: 白色隨機閃爍
// 2. 燈帶2: 紅色呼吸效果
// 3. 燈帶3: 動態彩虹流動

void loop(){
//HW5
    dynamicRainbow(leds_RGB3,NUM_RGB3,&starthue,5);
//HW6    
    randomFlash(leds_RGB1,NUM_RGB1,CRGB::White,50);
//HW7
    breathEffect(leds_RGB2,NUM_RGB2,HUE_RED,50);

//HW9
    waveEffect(leds_RGB1,NUM_RGB1,waterwave_palette);
}

//1️⃣ RGB全條白色長著
// void loop(){
//     fill_solid(leds_RGB1, NUM_RGB1, CRGB::White);
//     FastLED.show();
//     delay(500);
// }


//2️⃣RGB靜態彩虹
// void loop(){
//     fill_rainbow(leds_RGB1, NUM_RGB1, startIndext, 5);
//     FastLED.show();
//     delay(500);
// }
// 3️⃣1-6號單色燈：隨機亂閃
//Random Blinking
// void loop(){
    // digitalWrite(LED_COUNT[random(LED_PIN)],isOn ? HIGH:LOW );
    // isOn = !isOn;    
    // delay(flash);
// }


// 4️⃣RGB動態彩虹
// void loop(){
//     fill_rainbow(leds_RGB1, NUM_RGB1, startIndext, 5);
//     FastLED.show();
//     delay(50);
//     startIndext += 5;
// }

    // review1: 過度使用delay()會造成所有燈效卡住
    // 建議用millis()

// review2: 建議整個project集中一處call FastLED.show(), 減少燈效閃爍
// review2: 同樣，delay()也可集中在一處使用
