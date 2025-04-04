#pragma once

#include <stdint.h>

struct ButtonState {
    bool clicked : 1;
    bool long_click : 1;
    bool repeats: 4;
};

uint16_t getEncoderValue();
ButtonState getButtonState();

void setupEncoder();