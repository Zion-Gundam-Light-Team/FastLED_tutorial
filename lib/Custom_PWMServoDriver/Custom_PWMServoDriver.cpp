#include <Adafruit_PWMServoDriver.h>
#include "Custom_PWMServoDriver.h"

bool Custom_PWMServoDriver::setPWM_all(uint16_t onValues[16], uint16_t offValues[16])
{
  if (!this->i2c_dev)
    return false;
  uint8_t buffer[65];
  buffer[0] = PCA9685_LED0_ON_L;

  for (int i = 0; i < 16; i++)
  {
    buffer[1 + i * 4] = onValues[i] & 0xFF;
    buffer[2 + i * 4] = (onValues[i] >> 8) & 0xFF;
    buffer[3 + i * 4] = offValues[i] & 0xFF;
    buffer[4 + i * 4] = (offValues[i] >> 8) & 0xFF;
  }
  return this->i2c_dev->write(buffer, 65);
}