#include <FastLED.h>
#include <Adafruit_PWMServoDriver.h>
#include "Custom_PWMServoDriver.h"
#include "../../include/globals.h"
#include "../../include/pwmConfig.h"
#include "../../include/patterns/patterns_pwm.h"

static uint8_t flashingRandomPwm = 0;
static uint8_t flashingRandomPin = 0;

#define rangeOfVent 16
uint16_t ledFrequencies[rangeOfVent];
uint16_t ledMaxBrightness[rangeOfVent]; // Upper bounds (a values)
uint16_t ledMinBrightness[rangeOfVent]; // Lower bounds (b values)
static unsigned long lastRandomizeTime = 0;
uint16_t pwmVent(int patternNum, uint16_t MIN_FREQ, uint16_t MAX_FREQ, uint16_t lowerBoundOfMin, uint16_t upperBoundOfMin, uint16_t lowerBoundOfMax, uint16_t upperBoundOfMax, int intervals, unsigned long duration) {   
    unsigned long currentMillis = millis();      
    if (lastRandomizeTime < intervals && lastRandomizeTime == 0){    
        randomizeParameters(MIN_FREQ, MAX_FREQ, lowerBoundOfMin, upperBoundOfMin, lowerBoundOfMax, upperBoundOfMax);}    
        lastRandomizeTime = currentMillis;        
        if((lastRandomizeTime-currentMillis)% intervals ==0 && lastRandomizeTime < duration) {        
            randomizeParameters(MIN_FREQ, MAX_FREQ, lowerBoundOfMin, upperBoundOfMin,lowerBoundOfMax, upperBoundOfMax); }    
            lastRandomizeTime = currentMillis;  
            int brightness = beatsin8(ledFrequencies[patternNum], ledMinBrightness[patternNum], ledMaxBrightness[patternNum]);    
            return brightness;}
            
void randomizeParameters(int MIN_FREQ, int MAX_FREQ, int lowerBoundOfMin, int upperBoundOfMin, int lowerBoundOfMax, int upperBoundOfMax) {  
    randomSeed(millis());    
    for(int i = 0; i < 16; i++) {    
        ledFrequencies[i] = random8(MIN_FREQ, MAX_FREQ);    
        ledMinBrightness[i] = random8(lowerBoundOfMin, upperBoundOfMin);    
        ledMaxBrightness[i] = random8(lowerBoundOfMax, upperBoundOfMax);      
    }
}