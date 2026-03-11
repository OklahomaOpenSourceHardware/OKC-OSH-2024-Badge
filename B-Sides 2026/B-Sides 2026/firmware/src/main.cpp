#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <avr/sleep.h>
#include <avr/io.h>

// Using confirmed PIN_Pxn naming from btbm.ch
#define PIEZO_PIN   PIN_PA1
#define TOUCH_PIN   PIN_PA4
#define LED_DATA    PIN_PB4
#define LED_PWR_EN  PIN_PA7
#define NUM_LEDS    4

Adafruit_NeoPixel strip(NUM_LEDS, LED_DATA, NEO_GRB + NEO_KHZ800);

// Updated VDD Read from btbm.ch notes
long readVcc() {
  analogReference(INTERNAL1V1);
  // ADC_VDDDIV10 reads VDD/10. 


  // Result of 1024 at 10-bit = 1.024V (meaning VDD is 10.24V)

// We use the direct register constant confirmed by your grep
    // ADC_MUXPOS_VDDDIV10_gc is the raw hardware bitmask for VDD/10
    ADC0.MUXPOS = ADC_MUXPOS_AIN10_gc;
    ADC0.CTRLA = ADC_ENABLE_bm;
    ADC0.COMMAND = ADC_STCONV_bm;
while(!(ADC0.INTFLAGS & ADC_RESRDY_bm));
    uint16_t reading = ADC0.RES;

  if (reading == 0) return 0;
    // 1.1V * 1024 * 10 = 11264000
    return (11264000L / reading);
 // int32_t vdd = analogReadEnh(ADC_VDDDIV10, 12); 
//  vdd >>= 2; // Adjust for 12-bit to 10-bit
//  return vdd * 10; // Returns millivolts
}

void setLedPower(bool on) {
  if (on) {
    pinMode(LED_PWR_EN, OUTPUT);
    digitalWrite(LED_PWR_EN, LOW); 
    delay(10); 
    strip.begin();
    strip.show();
  } else {
    digitalWrite(LED_PWR_EN, HIGH);
    pinMode(LED_DATA, INPUT); 
  }
}

void playStarTrekChirp() {
  setLedPower(true);
  for (int j = 0; j < 2; j++) {
    int startFreq = (j == 0) ? 800 : 1000;
    for (int i = 0; i < 20; i++) {
      int freq = startFreq + (i * 20);
      tone(PIEZO_PIN, freq);
      for(int l=0; l<NUM_LEDS; l++) strip.setPixelColor(l, 0, 0, i * 12);
      strip.show();
      delay(5);
    }
    noTone(PIEZO_PIN);
    if (j == 0) delay(40);
  }
  setLedPower(false);
}

void setup() {
  // Enable WDT for 2s wakeup as seen in btbm.ch code
  _PROTECTED_WRITE(WDT.CTRLA, WDT_PERIOD_2KCLK_gc);
  
  pinMode(LED_PWR_EN, OUTPUT);
  setLedPower(false);
  pinMode(PIEZO_PIN, OUTPUT);
  pinMode(TOUCH_PIN, INPUT_PULLUP);
}

void loop() {
  // Direct reading of the touch pin
  // If you are using the PTC hardware, touchRead(TOUCH_PIN) is still preferred
  // but if the library fails to link, this pull-up method works:
  if (digitalRead(TOUCH_PIN) == LOW) { 
    playStarTrekChirp();
  }

  // Low power sleep
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();
  sleep_disable();
}
