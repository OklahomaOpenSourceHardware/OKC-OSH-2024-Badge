#include <Arduino.h>


#include "leds.h"
#include "phototrans.h"
#include "encoder.h"
#include "screen.h"
#include "utils.h"


DefaultScreen defaultScreen;
AnimationScreen animationScreen;

BreathingScreen breathingScreen;

TestScreen testScreen;

void setup()
{
 

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
    if (btn.long_click || btn.long_hold)
    {
      defaultScreen.select();
    }
    else if (defaultScreen.isActive())
    {
      switch (defaultScreen.level)
      {
      case 1:
        defaultScreen.select();
        break;
      case 2:
        testScreen.select();
        break;
      case 3:
        breathingScreen.select();
        break;
      case 4:
      breathingScreen.select();
        break;

      case 5:
        animationScreen.setPattern1();
        animationScreen.select();
        break;
        case 6:
        defaultScreen.select();
        break;
        case 7:
        defaultScreen.select();
        break;
        case 8:
        defaultScreen.select();
        break;
        case 9:
        defaultScreen.select();
        break;
        case 10:
        defaultScreen.select();
        break;
        case 11:
        defaultScreen.select();
        break;

      default:
        defaultScreen.select();
        break;
      }
    }
  }
}

void loop()
{
  handleAdc();
  handleButton();


  ScreenBase::executeCurrent();
}
