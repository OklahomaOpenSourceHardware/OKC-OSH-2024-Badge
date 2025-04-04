#pragma once

#include <stdint.h>

#define SSD1306_I2C_ADDR 0x3c
#define MPU6050_I2C_ADDR 0x68

void setupI2Cm();

void scanI2C();

bool hasAccel();
bool readAccel(int16_t* data);

bool hasOled();

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ssd1306Send(uint8_t *data, uint8_t sz);

#ifdef __cplusplus
}
#endif
