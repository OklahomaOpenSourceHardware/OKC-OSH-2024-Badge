#include "leds.h"

#include <Arduino.h>
#include <HardwareTimer.h>

#include "utils.h"

#define X1 PC4
#define X2 PC5
#define X3 PC6
#define X4 PC7

HardwareTimer timer(TIM2);

const uint32_t UPDATE_INTERVAL = 300;

void show_value();

void timerCallback()
{
  show_value();
  timer.resume();
}

void setupLit()
{
  configPinInput(X1);
  configPinInput(X2);
  configPinInput(X3);
  configPinInput(X4);

  timer.setOverflow(UPDATE_INTERVAL, MICROSEC_FORMAT);
  timer.attachInterrupt(timerCallback);
  timer.resume();
}

int pins[] = {X1, X2, X3, X4};

int l1 = 0, l2 = 0;

void noLit()
{
  if (!l1 && !l2)
  {
    return;
  }
  configPinInput(pins[l1]);
  configPinInput(pins[l2]);
  l1 = l2 = 0;
}

void lit(int i)
{
  if (l1 || l2)
  {
    configPinInput(pins[l1]);
    configPinInput(pins[l2]);
  }
  l1 = (i / 3) % 4;
  l2 = i % 3;
  if (l2 >= l1)
    l2++;
  configPinOutput(pins[l1]);
  digitalWrite(pins[l1], LOW);
  configPinOutput(pins[l2]);
  digitalWrite(pins[l2], HIGH);
}

int lit_n = 0;
int lit_m = 1;

void show_value()
{
  int n = lit_n;
  if (n < lit_m)
  {
    lit_m = 1;
    if (n < lit_m)
    {
      noLit();
      return;
    }
  }
  while (!(n & lit_m))
    lit_m <<= 1;
  lit(__builtin_popcount(lit_m - 1));
  lit_m <<= 1;
}

void setLitValue(int n)
{
  lit_n = n;
}
