#pragma once

#include <stdint.h>

bool getPhase();
bool getPreamble();

void setupPhoto();
bool receive();

int receivedFrames();
void getFrames(uint16_t *frames);
