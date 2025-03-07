#include "i2c_mini.h"

#if OPT_I2C_SLAVE
#error "don't need i2c slave code"
#endif

#include <Arduino.h>

#include "utility/twi.h"

i2c_t I2Cm;
bool _hasAccel = false;

void setupI2Cm()
{
  I2Cm.sda = digitalPinToPinName(PC1);
  I2Cm.scl = digitalPinToPinName(PC2);
  i2c_custom_init(&I2Cm, 100000, I2C_AcknowledgedAddress_7bit, 0x2);
}

bool hasAccel()
{
  return _hasAccel;
}

int n = 1000;

void scanI2C()
{
  if (--n)
  {
    return;
  }
  if (!_hasAccel)
  {
    static uint8_t MPU6050Init[2] = {0x6B, 0};
    if (I2C_OK == i2c_master_write(&I2Cm, MPU6050_I2C_ADDR << 1, MPU6050Init, 2, 1))
    {
      _hasAccel = true;
    }
  }
  n = 100000;
}

bool errAccel()
{
  _hasAccel = false;
  return false;
}

bool readAccel(int16_t *data)
{
  if (!hasAccel()) { return false; }

  uint8_t t[6];
  t[0] = 0x3B;
  i2c_status_e e;
  e = i2c_master_write(&I2Cm, MPU6050_I2C_ADDR << 1, t, 1, 0);
  if (e != I2C_OK)
  {
    return errAccel();
  }
  e = i2c_master_read(&I2Cm, MPU6050_I2C_ADDR << 1, t, 6);
  if (e != I2C_OK)
  {
    return errAccel();
  }
  data[0] = (int16_t(t[0]) << 8) | t[1];
  data[1] = (int16_t(t[2]) << 8) | t[3];
  data[2] = (int16_t(t[4]) << 8) | t[5];
  return true;
}