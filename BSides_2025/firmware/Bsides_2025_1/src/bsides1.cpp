#include <Arduino.h>
#include "i2c_mini.h"

#include "leds.h"
#include "phototrans.h"
#include "encoder.h"
#include "screen.h"
#include "utils.h"

DefaultScreen defaultScreen;
AnimationScreen animationScreen;
AccelerometerScreen accelScreen;
TextScreen textScreen;
TestScreen testScreen;

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
    uint16_t frames[MAX_FRAMES];
    frames_len = receivedFrames();
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
    else if (defaultScreen.isActive())
    {
      switch (defaultScreen.level)
      {
      case 2:
        testScreen.select();
        break;
      case 5:
        animationScreen.setPattern1();
        animationScreen.select();
        break;
      case 7:
        if (hasAccel())
        {
          accelScreen.select();
        }
        break;
      case 8:
        if (hasOled())
        {
          textScreen.select();
        }
        break;
      }
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
