#include "i2c_mini.h"

#if OPT_I2C_SLAVE
#error "don't need i2c slave code"
#endif

#include <Arduino.h>

#include "utility/twi.h"

extern "C" void ssd1306Init(void);

i2c_t I2Cm;
bool _hasAccel = false;
bool _hasOled = false;

uint32_t last_scan;

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

bool hasOled()
{
  return _hasOled;
}

void scanI2C()
{
  if (millis() - last_scan < 3000)
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
#ifdef HAS_OLED  
  if (!_hasOled) {
    static uint8_t NoDATA[] = {};
    if (I2C_OK == i2c_master_write(&I2Cm, SSD1306_I2C_ADDR << 1, NoDATA, 0, 1)) {
      _hasOled = true;
      ssd1306Init();
    }
  }
#endif  
  last_scan = millis();
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

uint8_t ssd1306Send(uint8_t *data, uint8_t sz) {
  uint8_t res = i2c_master_write(&I2Cm, SSD1306_I2C_ADDR << 1, data, sz, 1);
  if (res != I2C_OK) {
    _hasOled = false;
    static uint8_t OffCMD[] = {0, 0xAE};
    i2c_master_write(&I2Cm, SSD1306_I2C_ADDR << 1, OffCMD, 2, 1);
  }
  return res;
}
