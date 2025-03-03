#include "leds.h"

#include <Arduino.h>
#include <HardwareTimer.h>

#define X1 PC4
#define X2 PC5
#define X3 PC6
#define X4 PC7

HardwareTimer timer(TIM2);

const uint32_t UPDATE_INTERVAL = 300;

void show_value();

void timerCallback() {
  show_value();
  timer.resume();
}

void setup_lit() {
  pinMode(X1, INPUT);
  pinMode(X2, INPUT);
  pinMode(X3, INPUT);
  pinMode(X4, INPUT);

  timer.setOverflow(UPDATE_INTERVAL, MICROSEC_FORMAT); 
  timer.attachInterrupt(timerCallback);
  timer.resume();
}

int pins[] = { X1, X2, X3, X4};

int l1 = 0, l2 = 0;

void no_lit() {
  if (!l1 && !l2) return;
  pinMode(pins[l1], INPUT);
  pinMode(pins[l2], INPUT);
  l1 = l2 = 0;
}

void lit(int i) {
  if (l1 || l2) {
    pinMode(pins[l1], INPUT);
    pinMode(pins[l2], INPUT);
  }
  l1 = (i / 3) % 4; l2 = i % 3;
  if (l2 >= l1) l2++;
  pinMode(pins[l1], OUTPUT);
  digitalWrite(pins[l1], LOW);
  pinMode(pins[l2], OUTPUT);
  digitalWrite(pins[l2], HIGH);
}


int lit_n = 0;
int lit_m = 1;

void show_value() {
  int n = lit_n;
  if (n < lit_m) {
    lit_m = 1;
    if (n < lit_m) {
      no_lit();
      return;   
    }
  }
  while (!(n & lit_m)) lit_m <<= 1;
  lit(__builtin_popcount(lit_m - 1));
  lit_m <<= 1;
}

void set_value(int n) {
  lit_n = n;
}
