#pragma once

#include <stdint.h>

// Reduces amount of code pulled by pinMode.
void configPinInput(uint32_t pin);
void configPinOutput(uint32_t pin);

int patan2(int y, int x);

const int MAX_FRAMES = 128;

struct FramesData {
    uint16_t count;
    uint16_t frames[MAX_FRAMES];
};
