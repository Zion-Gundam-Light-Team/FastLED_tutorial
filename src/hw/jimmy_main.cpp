#include <Arduino.h>
#include <FastLED.h>

// Section 1
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

#define LED_PIN1 1
#define LED_PIN2 2
#define LED_PIN3 3
#define LED_PIN4 4
#define LED_PIN5 5
#define LED_PIN6 6
#define LED_NUM 6

// bool 
bool lights[LED_NUM];

#define PIN_RGB1 6
#define PIN_RGB2 5
#define PIN_RGB3 4

#define NUM_RGB1 10
#define NUM_RGB2 10
#define NUM_RGB3 10

#define NUM_RGB_TO_FLASH 6

DEFINE_GRADIENT_PALETTE(heatmap_gp){
    0, 0, 0, 0,        // black
    128, 255, 0, 0,    // red
    200, 255, 255, 0,  // yellow
    255, 255, 255, 255 // white
};

DEFINE_GRADIENT_PALETTE(heatmap_gp2){
    //0, 0, 0, 0,        // black
    0, 0, 128, 255,  // blue
    128, 0, 153, 76,   // green
    255, 255, 255, 255 // white
};

CRGB leds_RGB1[NUM_RGB1];
CRGB leds_RGB2[NUM_RGB2];
CRGB leds_RGB3[NUM_RGB3];


uint8_t startIndex = 0; // Hue color of the first light bulb

CRGBPalette16 heatmapPalette = heatmap_gp; // build CRGBPalette16 object from RGB Palette
CRGBPalette16 heatmapPalette2 = heatmap_gp2; // build CRGBPalette16 object from RGB Palette

uint8_t currentBrightness = 0;
uint8_t randomIndex = 0;

void dynamicRainbow(CRGB *leds, int numLeds, uint8_t *hue, uint8_t speed);
void randomFlash(CRGB *leds, int numLeds, CRGB color, uint8_t chance);
void breathEffect(CRGB *leds, int numLeds, CRGB color, uint8_t speed);
void waveEffect(CRGB *leds, uint8_t *colorIndex, int NUM_LEDS);

CHSV rgb2Hsv(CRGB rgb){
    uint8_t r = rgb.r;
    uint8_t g = rgb.g;
    uint8_t b = rgb.b;
    uint8_t cmax = max({r, g, b});
    uint8_t cmin = min({r, g, b});
    uint8_t diff = cmax - cmin;
    int H = 0;
    if (diff == 0) {
        H = 0; // Undefined hue
    } else if (cmax == r) {
        H = ((g - b) * 60) / diff;
        if (H < 0) H += 360;
    } else if (cmax == g) {
        H = ((b - r) * 60) / diff + 120;
    } else if (cmax == b) {
        H = ((r - g) * 60) / diff + 240;
    }
    uint8_t h = (H * 255) / 360; // Convert to FastLED hue range
    uint8_t s = (cmax == 0) ? 0 : (diff * 255) / cmax; // Saturation
    uint8_t v = cmax; // Original value, will be overwritten
    return CHSV(h, s, v);
}



void setup()
{
    // init function!
    Serial.begin(115200); // start serial port!
    FastLED.addLeds<LED_TYPE, PIN_RGB1, COLOR_ORDER>(leds_RGB1, NUM_RGB1);
    FastLED.addLeds<LED_TYPE, PIN_RGB2, COLOR_ORDER>(leds_RGB2, NUM_RGB1);
    FastLED.addLeds<LED_TYPE, PIN_RGB3, COLOR_ORDER>(leds_RGB3, NUM_RGB3);
    FastLED.setBrightness(16); // set max brightness globally. Do not set to 255 for eye vision protection.

    // For Exercise 3.
    // pinMode(LED_PIN1, OUTPUT);
    // pinMode(LED_PIN2, OUTPUT);
    // pinMode(LED_PIN3, OUTPUT);
    // pinMode(LED_PIN4, OUTPUT);
    // pinMode(LED_PIN5, OUTPUT);
    // pinMode(LED_PIN6, OUTPUT);

    // for (int i = 0; i < LED_NUM; i++) // from 1 to 6.
    //     lights[i] = false;
}



void loop()
{
    // section 1
    // fill_solid(leds_RGB1, NUM_RGB1, CRGB::Red); // CRGB(255,0,0)
    // FastLED.show(); // use this to "apply"!
    // delay(500); // give some time to display the effect

    // section 2
    // fill_rainbow(leds_RGB1, NUM_RGB1, startIndex, 5); // 4th parameter: Hue difference!
    // FastLED.show(); // use this to "apply"!
    // delay(500); // give some time to display the effect.

    // startIndex += 5; // make the effect becomes dynamic!

    // NOTE: delay() will pause CPU execution and globally affect all light effects.
    // Prefer to use EVERY_N_MILLISECONDS() by FastLED to ensure the other effects are not affected.

    // section 3:
    // EVERY_N_MILLISECONDS(500){
    //     fill_rainbow(leds_RGB1, NUM_RGB1, startIndex, 5); // 4th parameter: Hue difference!
    //     FastLED.show(); // use this to "apply"!
    //     delay(500); // give some time to display the effect.
    //     startIndex += 5; // make the effect becomes dynamic!
    // }

    // section 4:

    // Additional : CRGBPalette16:
    /*
        Compute the color changes
        DEFINE_GRADIENT_PALETTE(heatmap_gp){
            <coordinate>, <R>, <G>, <B>
        }
    */

    // customized gradient color
    // fill_palette(leds_RGB1, NUM_RGB1, startIndex, 255 / NUM_RGB1, heatmapPalette, 255, LINEARBLEND);

    // section 5.
    // fill_solid(leds_RGB1, NUM_RGB1, CRGB::Red); // fill all lights first.
    // fadeToBlackBy(leds_RGB1, NUM_RGB1, 10); // fade out by 10
    // FastLED.show();
    // delay(100);

    // section 5, b
    // fill_solid(leds_RGB1, NUM_RGB1, CRGB::Black); // fill all lights first.
    // currentBrightness = beatsin8(10, 0, 255); // <frequency>, <min brightness>, <max brightness>
    // fill_solid(leds_RGB1, NUM_RGB1, CHSV(0,255,currentBrightness)); // use CHSV to separate color and brightness control.
    // FastLED.show(); // use this to "apply"!
    // delay(500); // give some time to display the effect

    // section 6
    // fill_solid(leds_RGB1, NUM_RGB1, CRGB::Black); // fill all lights first.

    // randomIndex = (0, NUM_RGB1);          // random choose a rgb light.
    // leds_RGB1[randomIndex] = CRGB::White; // assign white color to a random location
    // fadeToBlackBy(leds_RGB1, NUM_RGB1, 100);
    // FastLED.show(); // use this to "apply"!
    // delay(100);     // give some time to display the effect

    // Exercise.
    // Ex1. Light up entire RGB LED with white color
    // fill_solid(leds_RGB1, NUM_RGB1, CRGB::White); // CRGB(255,0,0)
    // FastLED.show(); // use this to "apply"!
    // delay(500); // give some time to display the effect

    // Ex2. RGB static rainbow effect.
    // section 2
    // fill_rainbow(leds_RGB1, NUM_RGB1, startIndex, 10); // 4th parameter: Hue difference! Try Hue difference = 5 / 10
    // FastLED.show(); // use this to "apply"!
    // delay(500); // give some time to display the effect.

    // Ex3. Randomly flash light bulb #1 - #6
    // EVERY_N_MILLISECONDS(100){
    //     uint8_t randIndex = random8(LED_NUM);

    //     //int randIndex = random(0, LED_NUM);
    //     for (int i = 0; i < LED_NUM; i++) // from 1 to 6.
    //         lights[i] = false;
        
    //     lights[randIndex] = true;

    //     for (int i=0; i < LED_NUM; i++)
    //         digitalWrite(i+1,lights[i]);

    // };

    // Extras: Ex.3 for RGB lights. 
    // fill_solid(leds_RGB1, NUM_RGB1, CRGB::Black); // fill all lights first.
    // randomIndex = random(0, NUM_RGB_TO_FLASH);          // random choose a rgb light from 0 to 5
    // leds_RGB1[randomIndex] = CRGB::White; // assign white color to a random location
    // fadeToBlackBy(leds_RGB1, NUM_RGB_TO_FLASH, 100);
    // FastLED.show(); // use this to "apply"!
    // delay(100);     // give some time to display the effect


    // Ex4. Dynamic RGB rainbow effect.
    // EVERY_N_MILLISECONDS(250){
    //     fill_rainbow(leds_RGB1, NUM_RGB1, startIndex, 15); // 4th parameter: Hue difference!
    //     FastLED.show(); // use this to "apply"!
    //     startIndex += 5; // make the effect becomes dynamic!
    // }

    // Exercise 5. Application of dynamicRainbow
    //dynamicRainbow(leds_RGB1,NUM_RGB1,&startIndex,50);  

    // Exercise 6. Create a random flash function
    //randomFlash(leds_RGB1,5,CRGB::White,128);

    // Exercise 7. breath effect function
    //breathEffect(leds_RGB1,NUM_RGB1,CRGB::GreenYellow, 100);

    // Exercise 8. Run three RGB together:
    // 1. White random blink
    // 2. Red breath lights
    // 3. Dynamic rainbow light

    //1. 
    //randomFlash(leds_RGB1,NUM_RGB1,CRGB::White,128);

    //2. 
    //breathEffect(leds_RGB2,NUM_RGB2,CRGB::Red, 100);

    //3. 
    //dynamicRainbow(leds_RGB3,NUM_RGB3,&startIndex,50);

    // Exercise 9. waveEffect
    // NOTE: Just try, not sure if this is good.
    EVERY_N_MILLISECONDS(250){
        uint8_t rdx = random8(0,255);
        waveEffect(leds_RGB1,&rdx,NUM_RGB1);
    };
    
    

    FastLED.show(); // use this to "apply"!
}

// exercise 5. write dynamicRainbow
void dynamicRainbow(CRGB *leds, int numLeds, uint8_t *hue, uint8_t speed){
    // notes: since the hue is a pointer, you should use *hue to do dereference
    EVERY_N_MILLISECONDS(speed){
        fill_rainbow(leds, numLeds, *hue, 15); // 4th parameter: Hue difference!
        startIndex += 15; // make the effect becomes dynamic!
        // review2: 應該用 (*hue) += 15
        // review2: better coding practise: 盡量將變數封裝在method裡，避免牽涉到外部的code
        // review2: encapsulation
    };
}

// Exercise 6. Create a random flash function
void randomFlash(CRGB *leds, int numLeds, CRGB color, uint8_t chance){
    EVERY_N_MILLISECONDS(10){
        //fill_solid(leds, numLeds, CRGB::Black); // fill all lights first.
        randomIndex = random(0, chance);          // random choose a rgb light from 0 to 5
        leds[randomIndex] = color; // assign white color to a random location
        fadeToBlackBy(leds, numLeds, 10); // 3rd variable: uint8 --> larger = faster, vice versa
        //FastLED.show(); // use this to "apply"!
    };
}

// Exercise 7. breath effect function
void breathEffect(CRGB *leds, int numLeds, CRGB color, uint8_t speed){
    // section 5, b
        EVERY_N_MILLISECONDS(speed){
        //fill_solid(leds, numLeds, CRGB::Black); // fill all lights first.
        currentBrightness = beatsin8(10, 0, 200); // <frequency>, <min brightness>, <max brightness>
        //currentBrightness = beatsin8()
        // convert the CRGB color to CHSV
        CHSV hsvColor = rgb2Hsv(color); //review1: 做法太迂迴
        hsvColor.v = currentBrightness;
        // review1: 可用這個（整條RGB燈帶）調光暗： nscale8(leds, numLeds, brightness);
        fill_solid(leds, numLeds, hsvColor); // use CHSV to separate color and brightness control.
        //FastLED.show(); // use this to "apply"!
    };
}

// Exercise 9. wave effect
// NOTE: Just try, not sure if this is good.
void waveEffect(CRGB *leds, uint8_t *colorIndex, int NUM_LEDS){

    uint8_t bright = 64;
    for (int i = 0; i < NUM_LEDS; i++){
        //uint8_t randIdx = random8(*colorIndex + i);
        *colorIndex = random8(0,255);
        leds[i] = ColorFromPalette(heatmapPalette2,*colorIndex,bright,LINEARBLEND);
    }
    (*colorIndex) += 1; // review3: 這行被overwrite, 永遠不會有效果
}

// review4: EVERY_N_MILLISECONDS是static, 有機會造成燈條之間的不同步，
// 建議用millis()
