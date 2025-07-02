#ifndef CUSTOM_PWMSERVODRIVER_H
#define CUSTOM_PWMSERVODRIVER_H
#include <Adafruit_PWMServoDriver.h>

class Custom_PWMServoDriver : public Adafruit_PWMServoDriver
{
public:
  using Adafruit_PWMServoDriver::Adafruit_PWMServoDriver; // Inherit constructors
  bool setPWM_all(uint16_t onValues[16], uint16_t offValues[16]);
};

#endif // CUSTOM_PWMSERVODRIVER_H