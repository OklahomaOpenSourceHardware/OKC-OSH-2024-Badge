#include <Arduino.h>
#include "i2c_mini.h"

#include "leds.h"
#include "phototrans.h"
#include "encoder.h"
#include "screen.h"

DefaultScreen defaultScreen;
AnimationScreen animationScreen;
AccelerometerScreen accelScreen;
TextScreen textScreen;

void setup()
{
  setupI2Cm();

  setupLit();
  setupPhoto();
  setupEncoder();

  ScreenBase::defaultScreen = &defaultScreen;
  defaultScreen.select();
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
    int frames_len = 0;
    uint16_t frames[15];
    frames_len = min(receivedFrames(), 15);
    getFrames(frames);
    animationScreen.setFrames(frames, frames_len);
    animationScreen.select();
  }
}

void handleButton()
{
  ButtonState btn = getButtonState();
  ScreenBase::buttonState = btn;
  if (btn.clicked)
  {
    if (btn.long_click)
    {
      defaultScreen.select();
    }
    else if (hasAccel())
    {
      accelScreen.select();
    }
    else if (hasOled())
    {
      textScreen.select();
    }
    else
    {
      animationScreen.setPattern1();
      animationScreen.select();
    }
  }
}

void loop()
{
  handleAdc();
  handleButton();
  scanI2C();

  ScreenBase::executeCurrent();
}
