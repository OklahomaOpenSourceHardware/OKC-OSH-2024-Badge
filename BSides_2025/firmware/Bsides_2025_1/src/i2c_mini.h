#pragma once

#include <stdint.h>

//#define SSD1306_I2C_ADDR 0x3c
#define MPU6050_I2C_ADDR 0x68

void setupI2Cm();

void scanI2C();

bool hasAccel();
bool readAccel(int16_t* data);