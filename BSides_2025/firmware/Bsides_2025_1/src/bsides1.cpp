#include <Arduino.h>

#include "leds.h"
#include "phototrans.h"
#include "encoder.h"
#include "screen.h"
#include "utils.h"

DefaultScreen defaultScreen;
AnimationScreen animationScreen;

BreathingScreen breathingScreen;
GameScreen gameScreen;
TestScreen testScreen;
TransmitScreen txScreen(animationScreen.framesDataPtr());

void setup()
{
  setupLit();
  setupPhoto();
  setupEncoder();

  ScreenBase::defaultScreen = &gameScreen;
  gameScreen.select();
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
    {
      int frames_len = 0;
      uint16_t frames[MAX_FRAMES];
      frames_len = receivedFrames();
      getFrames(frames);
      animationScreen.setFrames(frames, frames_len);
      animationScreen.select();
    }
    {
      // program Flash too
      Storage tmp = storage;
      tmp.frames = *animationScreen.framesDataPtr();
      updateStorage(tmp);
    }
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
      gameScreen.select();
    }
    else if (gameScreen.isActive())
    {
      // Reset button state to avoid duplicate click.
      ScreenBase::buttonState = getButtonState();

      switch (gameScreen.level)
      {
      case 0:
        //defaultScreen.select();
        gameScreen.addInput(0);
        gameScreen.select();
        break;
      case 1:
        //defaultScreen.select();
        gameScreen.addInput(1);
        gameScreen.select();
        break;
      case 2:
        //testScreen.select();
        gameScreen.addInput(2);
        gameScreen.select();
        break;
      case 3:
        //animationScreen.setFrames(frames, frames_len);
        //animationScreen.select();
        gameScreen.addInput(3);
        gameScreen.select();
        break;
      case 4:
        //breathingScreen.select();
        gameScreen.addInput(4);
        gameScreen.select();
        break;
      case 5:
        //animationScreen.setPattern1();
        //animationScreen.select();
        gameScreen.addInput(5);
        gameScreen.select();
        break;

      case 6:
        gameScreen.addInput(6);
        gameScreen.select();
      case 7:
        gameScreen.addInput(7);
        gameScreen.select();
      break;

      case 8:
        //txScreen.select();
        gameScreen.addInput(8);
        gameScreen.select();
        break;

      case 9:
        gameScreen.addInput(9);
        gameScreen.select();
        break;
      case 10:
        gameScreen.addInput(10);
        gameScreen.select();
        break;
      case 11:
        gameScreen.addInput(11);
        gameScreen.select();
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

  if (gameScreen.isActive() && gameScreen.isDone())
  {
    breathingScreen.select();
  }
}
