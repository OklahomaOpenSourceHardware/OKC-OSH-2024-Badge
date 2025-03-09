#include <Arduino.h>
#include "i2c_mini.h"

#include <math.h>

#include "leds.h"
#include "phototrans.h"
#include "encoder.h"
#include "ssd1306_mini.h"

enum Action
{
  NONE,
  ANIMATION,
  ACCEL,
} action;

void setup()
{
  setupI2Cm();

  setupLit();
  setupPhoto();
  setupEncoder();

  action = NONE;
}

int frames_len = 0;
uint16_t frames[20];

int next_frame;
uint32_t last_frame_t;
const uint32_t FRAME_RATE = 333;

void animateFrames()
{
  if (millis() - last_frame_t < FRAME_RATE)
  {
    return;
  }
  last_frame_t = millis();
  setLitValue(frames[next_frame]);
  next_frame = (next_frame + 1) % frames_len;
}

int ax = 0, ay = 0, az = 0;

int patan2(int y, int x)
{
  int m = (abs(y) + abs(x)) / 4;
  y /= m;
  x /= m;
  if (x > 0)
  {
    if (y < 0)
    {
      return abs(x) > abs(y) ? 11 : 10;
    }
    else if (y > 0)
    {
      return abs(x) > abs(y) ? 1 : 2;
    }
    else
    {
      return 0;
    }
  }
  else if (x < 0)
  {
    if (y < 0)
    {
      return abs(x) > abs(y) ? 7 : 8;
    }
    else if (y > 0)
    {
      return abs(x) > abs(y) ? 5 : 4;
    }
    else
    {
      return 6;
    }
  }
  else
  {
    return y > 0 ? 3 : 9;
  }
}

void accelerometerDemo()
{
  if (millis() - last_frame_t < 10)
  {
    return; // let mpu6050 refreshd
  }
  last_frame_t = millis();

  int16_t data[3];
  if (!readAccel(data)) {
    return;
  }
  const int DECAY = 192;
  ax = (DECAY * ax + (256 - DECAY) * data[0]) / 256;
  ay = (DECAY * ay + (256 - DECAY) * data[1]) / 256;
  az = (DECAY * az + (256 - DECAY) * data[2]) / 256;

  // angle of lowest part of badge
  int a = patan2(ay, -ax);
  // esimate size of the curve based on incline of the badge
  int s = 2 * min(3, patan2(2 * abs(az), abs(ax) + abs(ay))) + 1;
  int v = (1 << (s + 1)) - 1; // curve binary
  int o = (s/2 + a) % 12; // ... and how to move it
  setLitValue(0xFFF & (v << (12 - o) | (v >> o)));
}

void handleAdc()
{
  if (getPreamble())
  {
    if (!receive())
    {
      delay(1000);
      return;
    }
    frames_len = min(receivedFrames(), 15);
    getFrames(frames);
    last_frame_t = millis();
    next_frame = 0;
    action = frames_len ? ANIMATION : NONE;
  }
}

void handleButton()
{
  ButtonState btn = getButtonState();
  if (btn.clicked)
  {
    if (btn.long_click)
    {
      frames_len = 0;
      action = NONE;
    }
    else if (hasAccel())
    {
      action = ACCEL;
    }
    else if (hasOled())
    {
      oledInit();
      oledDrawText(0,0, "Hello,", 1, 1);
      oledDrawText(20,16, "World!", 1, 1);
      oledRefresh();
    }
    else
    {
      frames_len = 2;
      frames[0] = 0b1010;
      frames[1] = 0b0101;
      action = ANIMATION;
    }
  }
}

void loop()
{
  handleAdc();
  handleButton();
  scanI2C();

  switch (action)
  {
  case ANIMATION:
    animateFrames();
    break;
  case ACCEL:
    accelerometerDemo();
    break;
  default:
    // Some demo
    int v1 = getEncoderValue(); // millis() / 100;
    uint32_t display = getPhase() ? 0x03F : 0xFC0;
    display |= (display << 12);
    setLitValue(0xFFF & (display >> (v1 % 12)));
    break;
  }
}
