#include <FastLED.h>
#include "Custom_PWMServoDriver.h"
#include "../include/globals.h"
#include "../include/palettes.h"
#include "../include/lib_effects.h"
#include "../include/patterns_pwm.h"

static uint8_t flashingRandomPwm = 0;
static uint8_t flashingRandomPin = 0;

Custom_PWMServoDriver *pwmArray[MAX_NUM_PWM] = {nullptr};
uint16_t pwmBuffer[MAX_NUM_PWM][16] = {0};

#define rangeOfVent 16

void initPwm()
{
    Serial.println("==initPWM...");
    Wire1.begin(PIN_PWM_SDA, PIN_PWM_SCL, 100000);
    uint8_t pwmAddresses[MAX_NUM_PWM] = {
        PWM_ADDRESS_1,
        PWM_ADDRESS_2,
        PWM_ADDRESS_3,
        PWM_ADDRESS_4,
        PWM_ADDRESS_5,
        PWM_ADDRESS_6,
        PWM_ADDRESS_7,
        PWM_ADDRESS_8,
        PWM_ADDRESS_9,
        PWM_ADDRESS_10,
        PWM_ADDRESS_11,
        PWM_ADDRESS_12,
        PWM_ADDRESS_13,
        PWM_ADDRESS_14,
        PWM_ADDRESS_15,
        PWM_ADDRESS_16,
        PWM_ADDRESS_17,
        PWM_ADDRESS_18,
        PWM_ADDRESS_19,
        PWM_ADDRESS_20,
        PWM_ADDRESS_21,
        PWM_ADDRESS_22,
        PWM_ADDRESS_23,
        PWM_ADDRESS_24,
        PWM_ADDRESS_25};

    for (int i = 0; i < ACTUAL_NUM_PWM; i++)
    {
        Wire1.beginTransmission(pwmAddresses[i]);
        if (Wire1.endTransmission() == 0)
        {
            pwmArray[i] = new Custom_PWMServoDriver(pwmAddresses[i], Wire1);
            pwmArray[i]->begin();
            pwmArray[i]->setPWMFreq(PWM_FREQUENCY);
            Serial.print("==SUCCESS init pca9685: 0x");
            Serial.println(pwmAddresses[i], HEX);
        }
        else
        {
            pwmArray[i] = nullptr;
            Serial.print("==FAIL init pca9685: 0x");
            Serial.println(pwmAddresses[i], HEX);
        }
    }
}

uint16_t ledFrequencies[rangeOfVent];
uint16_t ledMaxBrightness[rangeOfVent]; // Upper bounds (a values)
uint16_t ledMinBrightness[rangeOfVent]; // Lower bounds (b values)
static unsigned long lastRandomizeTime = 0;

//散氣口
uint16_t pwmVent(int patternNum, uint16_t MIN_FREQ, uint16_t MAX_FREQ, uint16_t lowerBoundOfMin, uint16_t upperBoundOfMin, uint16_t lowerBoundOfMax, uint16_t upperBoundOfMax, int intervals, unsigned long duration) {
  
  // Check if 10 seconds have passed to randomize parameters
  static unsigned long currentMillis = millis();
  
    if (lastRandomizeTime < intervals && lastRandomizeTime == 0){
    randomizeParameters(MIN_FREQ, MAX_FREQ, lowerBoundOfMin, upperBoundOfMin, lowerBoundOfMax, upperBoundOfMax);}

    lastRandomizeTime = currentMillis;
    
    if((lastRandomizeTime-currentMillis)% intervals ==0 && lastRandomizeTime < duration) {
        randomizeParameters(MIN_FREQ, MAX_FREQ, lowerBoundOfMin, upperBoundOfMin,lowerBoundOfMax, upperBoundOfMax);
    }
    lastRandomizeTime = currentMillis;

  // Generate current brightness value
  static int brightness = beatsin8(ledFrequencies[patternNum], ledMinBrightness[patternNum], ledMaxBrightness[patternNum]);
  
  return brightness;
}

void randomizeParameters(int MIN_FREQ, int MAX_FREQ,
                        int lowerBoundOfMin, int upperBoundOfMin,
                        int lowerBoundOfMax, int upperBoundOfMax) {
  randomSeed(millis()); // Reseed for better randomness
  
  for(int i = 0; i < 16; i++) {
    ledFrequencies[i] = random8(MIN_FREQ, MAX_FREQ);
    ledMinBrightness[i] = random8(lowerBoundOfMin, upperBoundOfMin);
    ledMaxBrightness[i] = random8(lowerBoundOfMax, upperBoundOfMax);
    
  }
}



void pwmOnAll(uint16_t pwmBuffer[][16], uint16_t brightness)
{
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
        for (int channel = 0; channel < 16; channel++)
            pwmBuffer[pwmIndex][channel] = brightness;
}

void pwmOffAll(uint16_t pwmBuffer[][16])
{
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
        for (int channel = 0; channel < 16; channel++)
            pwmBuffer[pwmIndex][channel] = 0;
}

uint16_t pwmOff()
{
    return 0;
}

uint16_t pwmOn(uint16_t brightness)
{
    return brightness;
}

void pwmBreathAll(uint16_t pwmBuffer[][16], int freq, uint16_t brightnessLow, uint16_t brightnessHigh)
{
    uint16_t breath_brightness = beatsin16(freq, brightnessLow, brightnessHigh);
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
        for (int channel = 0; channel < 16; channel++)
            pwmBuffer[pwmIndex][channel] = breath_brightness;
}

bool pwmFadeOutAll(uint16_t pwmBuffer[][16], int fadeSpeed, uint16_t &currentBrightness)
{
    bool allOff = true;
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
    {
        currentBrightness = (currentBrightness * (255 - fadeSpeed)) / 255;
        for (int channel = 0; channel < 16; channel++)
            pwmBuffer[pwmIndex][channel] = currentBrightness;
        if (currentBrightness != 0)
            allOff = false;
    }
    return allOff;
}

uint16_t pwmFadeIn(unsigned long &currentTime, int fadeSpeed, uint16_t brightnessHigh, uint16_t &currentBrightness, unsigned long &lastUpdate)
{
    if (currentBrightness < brightnessHigh)
    {
        if (currentTime - lastUpdate >= fadeSpeed)
        {
            currentBrightness++;
            lastUpdate = currentTime;
            return currentBrightness;
        }
    }
    return brightnessHigh;
}

uint16_t pwmFadeOut(int fadeSpeed, uint16_t &currentBrightness)
{
    if (currentBrightness > fadeSpeed)
        currentBrightness -= fadeSpeed;
    else
        currentBrightness = 0;
    return currentBrightness;
}


uint16_t pwmfadeOut1(int fadeSpeed, uint16_t brightnessHigh, uint16_t brightnessLow, int stopSecond, unsigned long &lastUpdateArr_pwm)
{
    static uint16_t scaledBrightness;
    static uint16_t currentBrightness = brightnessHigh;
    //static unsigned long lastUpdate = 0;
    static bool inStopPhase = false;
    unsigned long startTiming = millis();

    if (!inStopPhase) {
        if (startTiming - lastUpdateArr_pwm >= 0) {
            lastUpdateArr_pwm = startTiming;

            if (currentBrightness > brightnessLow) {
                currentBrightness -= fadeSpeed;
		        scaledBrightness = currentBrightness;
            } else {
                scaledBrightness = brightnessLow;
                inStopPhase = true;
                lastUpdateArr_pwm = startTiming;
            }
        }
    } else {
        if (startTiming - lastUpdateArr_pwm >= stopSecond) {
            currentBrightness = brightnessHigh;
            scaledBrightness = currentBrightness;
            inStopPhase = false;
            lastUpdateArr_pwm = startTiming;
        }
    }
    return scaledBrightness;
}

uint16_t pwmFlash(uint16_t brightness, int bpm, unsigned long &lastToggle, bool &isOn)

{
    uint16_t scaledBrightness;
    unsigned long interval = 60000 / bpm;
    if (millis() - lastToggle >= interval)
    {
        scaledBrightness = isOn ? 0 : brightness;
        isOn = !isOn;
        lastToggle = millis();
    }
    return scaledBrightness;
}

uint16_t pwmFlashIdle(uint16_t brightness, int speed, uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate)
{
    int interval = speed;
    if (flashCount < 12)
    {
        if (millis() - lastUpdate >= interval)
        {
            lastUpdate = millis();
            return isOn ? 0 : brightness;
            isOn = !isOn;
            flashCount++;
        }
    }
    else
    {
        if (millis() - lastUpdate >= 3000)
        {
            flashCount = 0;
            isOn = false;
            lastUpdate = millis();
        }
    }
    return brightness;
}

uint16_t pwmBreath(int freq, uint16_t brightnessLow, uint16_t brightnessHigh)
{
    uint16_t breathBrightness = beatsin16(freq, brightnessLow, brightnessHigh);
    return breathBrightness;
}

uint16_t pwmBreathFlash(int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    breath_brightness = beatsin8(freq, 3, 210);
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        isOn = !isOn;
    }
    return isOn ? breath_brightness : 0;
}

uint16_t pwmBreathFlash1(int freq, uint16_t brightnessLow, uint16_t brightnessHigh, int breathFlashSecond, int breathFlashCount, int countInterval, bool &startRun, int stopSecond, bool &isOn)
{
    static bool inCountInterval = false;
    static int count = 0;
    unsigned long startTiming = millis();    
    static unsigned long flashTiming = 0;
    static unsigned long countIntervalTiming = 0;
    unsigned long flashInterval = 60000 / freq;
    static uint16_t breath_brightness;
    static uint16_t scaledBrightness;

    if (startRun) {
        if (inCountInterval) {
            if (startTiming - countIntervalTiming >= countInterval) {
                inCountInterval = false;
                countIntervalTiming = startTiming;
                flashTiming = startTiming;
            } else {
                scaledBrightness = 0;
                return 0;
            }
        }
        if (startTiming - flashTiming >= flashInterval) {
            flashTiming = startTiming;
            isOn = !isOn;
        }
        breath_brightness = beatsin16(freq, brightnessLow, brightnessHigh);
        scaledBrightness = isOn ? breath_brightness : 0;

        if (startTiming - countIntervalTiming >= breathFlashSecond) {
            count++;
            if (count >= breathFlashCount) {
                startRun = false;
                count = 0;
                countIntervalTiming = startTiming;
            } else {
                inCountInterval = true;
                countIntervalTiming = startTiming;
                scaledBrightness = 0;
            }
        }
    } else {
        if (startTiming - countIntervalTiming >= stopSecond) {
            startRun = true;
            countIntervalTiming = startTiming;
            inCountInterval = false;
        } else {
            scaledBrightness = 0;
        }
    }
	return scaledBrightness;
}

uint16_t pwmBreathFlashStop(int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    breath_brightness = beatsin16(freq, 3, 4000);
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        if (breath_brightness < 10)
            isOn = false;
        else
            isOn = !isOn;
    }
    return isOn ? breath_brightness : 0;
}

uint16_t pwmFlashRandom(int bpm, uint16_t brightnessLow, uint16_t brightnessHigh, bool &isOn, unsigned long &lastUpdate)
{
    uint16_t scaledBrightness;
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        uint16_t brightness = random16(brightnessLow, brightnessHigh);
        scaledBrightness = isOn ? 0 : brightness;
        isOn = !isOn;
    }
    return scaledBrightness;
}

uint16_t handle_Breath_Flash(int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    breath_brightness = beatsin16(freq, 3, 4000);
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        isOn = !isOn;
    }
    return isOn ? breath_brightness : 0;
}

uint16_t pwmHalfBreathFlash(int bpm, uint16_t &currentBrightness, bool &isOn, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        if (currentBrightness <= 0)
            currentBrightness = 4000;
        else
        {
            currentBrightness -= 10;
            isOn = !isOn;
        }
    }
    return isOn ? currentBrightness : 0;
}

uint16_t pwmValcanGun(uint16_t brightness, int speed, uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate, int pauseTime, int flashTime)
{
    unsigned long currentMillis = millis();
    
    // Flash phase (3 flashes)
    if (flashCount < 2*flashTime)  // 6 because each flash has ON and OFF states
    {
        if (currentMillis - lastUpdate >= speed)
        {
            lastUpdate = currentMillis;
            isOn = !isOn;
            flashCount++;
        }
    }
    // Pause phase (1000ms)
    else if (currentMillis - lastUpdate >= pauseTime)
    {
        flashCount = 0;  // Reset counter for next cycle
        isOn = false;    // Start with LED off
        lastUpdate = currentMillis;
    }
    
    return isOn ? brightness : 0;
}


uint16_t pwmHalfBreath(int bpm, uint16_t &currentBrightness, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        if (currentBrightness >= 200)
            currentBrightness = 0;
        else
            currentBrightness += 10;
    }
    return currentBrightness;
}

// 呼吸 + 停 呼吸信號燈
uint16_t pwmBreathStop(uint16_t brightnessLow, uint16_t brightnessHigh, int breathSecond, int breathCount, int stopSecond)
{
    static unsigned long totalCycleStart = 0;
    unsigned long startTiming = millis();
    unsigned long totalCycle = (breathCount * breathSecond + stopSecond);   //For accurately calculate breath in different duration, stopSecond or breathCount
    static uint16_t scaledBrightness;
    
    // reset totalCycle
    if (startTiming - totalCycleStart >= totalCycle) {
        totalCycleStart = startTiming;
        startTiming = millis();
    }

    unsigned long nBreathTimes = breathCount * breathSecond;
    
    if (startTiming - totalCycleStart < nBreathTimes) {
        unsigned long currentBreathTiming = (startTiming - totalCycleStart) % breathSecond;
        unsigned long halfTime = breathSecond / 2;
        uint16_t brightness;
        //Replace beatsin8(), It is difficult to calculate the time ratio between beatsin8() and stopSecond
        if (currentBreathTiming  <= halfTime) {
            brightness = map(currentBreathTiming, 0, halfTime, brightnessLow, brightnessHigh);
        } else {
            brightness = map(currentBreathTiming, halfTime, breathSecond, brightnessHigh, brightnessLow);
        }
        scaledBrightness = brightness;
    }
    else {
        scaledBrightness = brightnessLow;
        //pinMode(pin, OUTPUT);   //clear PIN setting (value) to make sure brightness is correct
    }
    return scaledBrightness;
}

uint16_t *pwmFlashRandomWithChance(uint16_t brightnessLow, uint16_t brightnessHigh, fract8 chance, bool isOnArr[16], unsigned long lastUpdateArr[16], int bpm)
{
    static uint16_t offValues[16];
    int interval = 60000 / bpm;
    for (int channel = 0; channel < 16; channel++)
    {
        if (millis() - lastUpdateArr[channel] >= interval)
        {
            lastUpdateArr[channel] = millis();
            if (random8() < chance)
            {
                if (!isOnArr[channel])
                {
                    uint16_t brightness = random16(brightnessLow, brightnessHigh);
                    offValues[channel] = brightness;
                    isOnArr[channel] = true;
                }
                else
                {
                    offValues[channel] = 0;
                    isOnArr[channel] = false;
                }
            }
            else
            {
                offValues[channel] = 0;
                isOnArr[channel] = false;
            }
        }
        else
        {
            if (isOnArr[channel])
                offValues[channel] = random16(brightnessLow, brightnessHigh);
            else
                offValues[channel] = 0;
        }
    }
    return offValues;
}

std::array<uint16_t, 2> pwmFlashAlternative(uint16_t brightness, int bpm, unsigned long &lastToggle, bool &isOn)
{
    unsigned long interval = 60000 / bpm;
    if (millis() - lastToggle >= interval)
    {
        isOn = !isOn;
        lastToggle = millis();
    }
    uint16_t scaledBrightness1 = isOn ? 0 : brightness;
    uint16_t scaledBrightness2 = isOn ? brightness : 0;
    return {scaledBrightness1, scaledBrightness2};
}

uint16_t pwm_led_flash_stop(uint16_t brightness, int flashSpeed, int stopSecond, int flashCcount, unsigned long &lastUpdate) {
  static int count = 0;
  static uint16_t scaledBrightness;

  if (count >= flashCcount * 2) {
    if (millis() - lastUpdate >= stopSecond) {
      count = 0;
      lastUpdate = millis();
    }
    return 0;
  }

  if (millis() - lastUpdate >= flashSpeed) {
    scaledBrightness = map((count % 2 == 0) ? brightness : 0, 0, 255, 0, 4095);
    count++;
    lastUpdate = millis();
  }
  return scaledBrightness;
}

uint16_t pwmFlashStop(uint16_t brightness, int flashSpeed, int stopSecond, int flashCount, unsigned long &lastUpdate) 
{
  static int count = 0;
  static bool isOn = false;
  static uint16_t scaledBrightness;

  if (count >= flashCount * 2) { 
    scaledBrightness = 0;
    if (millis() - lastUpdate >= stopSecond) {
      count = 0;
      lastUpdate = millis();
    }
    return 0;
  }

  if (millis() - lastUpdate >= flashSpeed) {
    scaledBrightness = (count % 2 == 0) ? brightness : 0;
    count++;
    lastUpdate = millis();
  }
return scaledBrightness;
}

uint16_t pwmProgressiveFlash(uint16_t brightnessHigh, uint16_t brightnessLow, int totalDuration, int stopSecond)
{
    static unsigned long lastUpdate = 0;
    static unsigned long currentDelay = 0;
    static bool isOn = false;
    static unsigned long finishTime = 0;
    static unsigned long startTime = 0;
    unsigned long currentTime = millis();
    static uint16_t scaledBrightness;

    if (currentTime - lastUpdate >= currentDelay) {
        if (currentTime - startTime >= totalDuration) {
            scaledBrightness = brightnessLow;

            if (finishTime == 0) {
              finishTime = currentTime;
            } else if (currentTime - finishTime >= stopSecond) {
              startTime = currentTime;
              finishTime = 0;
              isOn = false;
              currentDelay = 0;
            }
        return 0;
        }
        
        float timeProgress = (float)(currentTime - startTime) / totalDuration;  
        float easedProgress = pow(timeProgress, 0.2);

        uint16_t brightness = brightnessHigh - (uint16_t)((brightnessHigh - brightnessLow) * easedProgress);
        currentDelay = max(2, (int)(100 * (1.0 - easedProgress)));
        
        scaledBrightness = isOn ? brightnessLow : brightness;
        isOn = !isOn;
        
        lastUpdate = currentTime;
    }
    return scaledBrightness;
}

bool randomLightup(
    uint16_t pwmBuffer[][16],
    int maxNumPwm,
    int slaveId,
    unsigned long offDuration,
    int flashingSpeed,
    uint16_t minBrightness,
    uint16_t maxBrightness,
    bool isOnArr_pwm[][16],
    unsigned long lastUpdate_pwm[][16])
{
    static int currentPwmIndex = 0;
    static int flashingRandomChannel;
    static bool isFlashing = false;
    static unsigned long lastFlashTime = 0;

    // hardcode 亮點的順序。 如果slave 數目唔同，請自行在此更改。
    const int slaveIdSequences[][6] = {
        {1, 2, 3, 1, 4, 2},
        {4, 2, 3, 2, 2, 1},
        {2, 3, 4, 1, 2, 1},
        {1, 1, 4, 2, 3, 2},
    };

    const int numSequences = sizeof(slaveIdSequences) / sizeof(slaveIdSequences[0]);
    const int sequenceLength = sizeof(slaveIdSequences[0]) / sizeof(slaveIdSequences[0][0]);

    static int currentSequenceIndex = 0;
    static int currentSlaveIndex = 0;

    for (int i = 0; i < maxNumPwm; i++)
    {
        for (int j = 0; j < 16; j++)
            pwmBuffer[i][j] = 0;
    }
    if (!isFlashing)
    {
        if (millis() - lastFlashTime >= offDuration)
        {
            currentSequenceIndex = random(0, numSequences);
            currentSlaveIndex = 0;
            if (slaveId == slaveIdSequences[currentSequenceIndex][currentSlaveIndex])
            {
                currentPwmIndex = random(0, maxNumPwm);
                flashingRandomChannel = random8(0, 16);
                isFlashing = true;
            }
        }
    }
    if (isFlashing)
    {
        pwmBuffer[currentPwmIndex][flashingRandomChannel] = pwmFlashRandom(
            flashingSpeed,
            minBrightness,
            maxBrightness,
            isOnArr_pwm[currentPwmIndex][flashingRandomChannel],
            lastUpdate_pwm[currentPwmIndex][flashingRandomChannel]);

        if (pwmBuffer[currentPwmIndex][flashingRandomChannel] == 0)
        {
            isFlashing = false;
            lastFlashTime = millis();
            currentSlaveIndex = (currentSlaveIndex + 1) % sequenceLength;
        }
    }
    return isFlashing;
}

void dispatchPwm()
{
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
    {
        Custom_PWMServoDriver *pwm = pwmArray[pwmIndex];
        if (pwm == nullptr)
            continue;

        uint16_t onValues[16] = {0};
        uint16_t offValues[16];
        for (int channel = 0; channel < 16; channel++)
        {
            offValues[channel] = pwmBuffer[pwmIndex][channel];
        }
        pwm->setPWM_all(onValues, offValues);
    }
}

//
uint16_t pwmFlashKeep(uint16_t brightness, float flashCount, int flashDurationMs, int keepOnMillis, unsigned long &lastUpdate) 
{
  static enum { KEEP_ON, FLASHING } state = KEEP_ON;
  static unsigned long stateStart = 0;
  static int count = 0;
  unsigned long now = millis();
  uint16_t scaledBrightness;

  int flashSpeed = flashDurationMs / (flashCount * 2);

  if (state == KEEP_ON) {
    scaledBrightness = brightness;
    if (now - stateStart >= keepOnMillis) {
      state = FLASHING;
      stateStart = now;
      count = 0;
      lastUpdate = now;
    }

  } else if (state == FLASHING) {
    if (now - stateStart >= flashDurationMs) {
      state = KEEP_ON;
      stateStart = now;
      scaledBrightness = brightness;
      return 0;
    }

    if (now - lastUpdate >= flashSpeed) {
      scaledBrightness = (count % 2 == 0) ? brightness : 0;
      count++;
      lastUpdate = now;
    }
  }
  return scaledBrightness;
}