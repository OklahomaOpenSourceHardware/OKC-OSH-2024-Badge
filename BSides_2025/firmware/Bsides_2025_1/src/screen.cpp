#include "screen.h"

#include <Arduino.h>

#include "leds.h"
#include "encoder.h"
#include "phototrans.h"
#include "utils.h"

static const int FRAMES_LEN = 64; // Reduce from MAX_FRAMES to 64

void ScreenBase::enter() {}
void ScreenBase::execute() {}
void ScreenBase::leave() {}

ScreenBase *ScreenBase::current = nullptr;
ScreenBase *ScreenBase::defaultScreen = nullptr;
ButtonState ScreenBase::buttonState;

void DefaultScreen::enter()
{
  lastEncoder = getEncoderValue();
}

void DefaultScreen::execute()
{
  uint16_t enc = getEncoderValue();
  int delta = -int16_t(enc - lastEncoder) / 2;
  if (delta != 0)
  {
    lastEncoder = enc;
    level = max(1, min(11, level + delta));
  }
  int pattern = (1 << level) - 1;
  pattern = (pattern << 2) | (pattern >> 10);
  setLitValue(pattern);
}

void TestScreen::execute()
{
  int v1 = getEncoderValue(); // millis() / 100;
  uint32_t display = getPhase() ? 0x03F : 0xFC0;
  display |= (display << 12);
  setLitValue(0xFFF & (display >> (v1 % 12)));
}

void StaticAnimationScreen::enter()
{
  last_frame_t = millis();
  next_frame = 0;
}

void StaticAnimationScreen::execute()
{
  if (millis() - last_frame_t < FRAME_RATE)
  {
    return;
  }
  if (next_frame >= framesCount())
  {
    next_frame = 0;
  }
  last_frame_t = millis();
  setLitValue(frame(next_frame));
  next_frame++;
}

void AnimationScreen::enter()
{
  last_frame_t = millis();
  next_frame = 0;
}

void AnimationScreen::execute()
{
  if (millis() - last_frame_t < FRAME_RATE)
  {
    return;
  }
  last_frame_t = millis();
  setLitValue(framesData.frames[next_frame]);
  next_frame = (next_frame + 1) % framesData.count;
}

void AnimationScreen::setFrames(uint16_t *frames, int count)
{
  framesData.count = count;
  memcpy(framesData.frames, frames, count * sizeof(frames[0]));
  next_frame = 0;
}

void TransmitScreen::enter()
{
  bitIndex = 0;
  last_t = millis() - BIT_RATE - 1;
}

void TransmitScreen::execute()
{
  if (millis() - last_t < BIT_RATE)
  {
    return;
  }
  int byteIndex = bitIndex >> 4;
  uint8_t byte;
  if (byteIndex < 3) {
    if (byteIndex == 2)
      byte = framesData->count * 2;
    else
      byte = byteIndex == 0 ? 0x55 : 0xD5; 
  } else {
    int frameIndex = (byteIndex - 3) >> 1;
    if (frameIndex >= framesData->count) {
      // end of transmit
      setLitValue(0);
      defaultScreen->select();
      return;
    }
    byte = (byteIndex & 1) ? framesData->frames[frameIndex] : (framesData->frames[frameIndex] >> 8);
  }
  bool f = !(bitIndex & 1);
  int bit = (bitIndex >> 1) & 7;
  bool pos = (((byte >> bit) & 1) != 0) == f;
  setLitValue(pos ? 0b111000 : 0b111000000000);
  bitIndex++;
  last_t = millis();
}
 

void GameScreen::enter()
{
  startLed = 1;
  lastEnc = getEncoderValue();
  memset(steps, 42, MAX_STEPS);
}

void GameScreen::execute()
{
  int v = max(-11, min(11, int(int16_t(lastEnc - getEncoderValue()))));
  int pattern = (1 << abs(v)) - 1;
  int shift = (v >= 0 ? startLed + 1 : startLed + 12 + v) % 12;
  setLitValue(((pattern >> (12 - shift)) | (pattern << shift)) & 0xFFF);

  if (buttonState.clicked)
  {
    addInput(v);
    startLed += v;
    lastEnc = getEncoderValue();
  }
}

void GameScreen::addInput(int step)
{
  memmove(steps + 1, steps, MAX_STEPS - 1);
  steps[0] = step;
}

bool GameScreen::isDone() const
{
  return steps[0] == 5 && steps[1] == -2 && steps[2] == 0 && steps[3] == 2;
}

int BreathingScreen::framesCount() const
{
  return 22 * 4;
}

uint16_t BreathingScreen::frame(int i) const
{
  i >>= 2;
  int len = (i < 11) ? i : (22 - i);
  return ((1 << len) - 1) << 2;
}
