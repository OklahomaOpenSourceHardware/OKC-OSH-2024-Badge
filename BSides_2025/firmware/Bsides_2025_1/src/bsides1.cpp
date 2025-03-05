#include <Arduino.h>
#include <Wire.h>

#include "leds.h"
#include "phototrans.h"
#include "encoder.h"

void setup() {
  setup_lit();
  setup_photo();
  setup_encoder();
}

void loop() {
  if (getPreamble()) {
    set_value(0b1);
    receive(); return;
  }
  int v1 = getEncoderValue(); // millis() / 100;
  uint32_t display = getPhase() ? 0x03F : 0xFC0; display |= (display << 12);
  set_value(0xFFF & (display >> (v1 % 12)) );
}
