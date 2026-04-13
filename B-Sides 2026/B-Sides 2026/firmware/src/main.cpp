#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <ptc.h>

// ---------------------------------------------------------
// DEBUG TOGGLE: Comment out this line to DISABLE deep sleep
// and timers to verify core functionality.
// ---------------------------------------------------------
//#define ENABLE_LOW_POWER

#define PIEZO_PIN   PIN_PA1
#define TOUCH_PIN   PIN_PA4
#define LED_DATA    PIN_PB4
#define LED_PWR_EN  PIN_PB5
#define SDA_SAO     PIN_PB2
#define SCL_SAO     PIN_PB3
#define PC3_SAO     PIN_PC3
#define NUM_LEDS    4

Adafruit_NeoPixel strip(NUM_LEDS, LED_DATA, NEO_GRB + NEO_KHZ800);
uint32_t nextSleep = 10000;
cap_sensor_t touch_node;

long readVcc() {
  analogReference(INTERNAL1V1);
  
  ADC0.MUXPOS = ADC_MUXPOS_AIN10_gc;
  ADC0.CTRLA = ADC_ENABLE_bm;
  ADC0.COMMAND = ADC_STCONV_bm;
  
  while(!(ADC0.INTFLAGS & ADC_RESRDY_bm));
  uint16_t reading = ADC0.RES;

  ADC0.CTRLA &= ~ADC_ENABLE_bm; 

  if (reading == 0) return 0;
  return (11264000L / reading);
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
    pinMode(LED_DATA, OUTPUT); 
    digitalWrite(LED_DATA, LOW);
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
    digitalWrite(PIEZO_PIN, LOW); 
    if (j == 0) delay(40);
  }
  setLedPower(false);
}

void ptc_event_cb_touch(const ptc_cb_event_t eventType, cap_sensor_t *node) {
  if (PTC_CB_EVENT_TOUCH_DETECT == eventType) {
    playStarTrekChirp();
  } 
}

void ptc_event_cb_calibration(const ptc_cb_event_t eventType, cap_sensor_t* node)  {
  if (PTC_CB_EVENT_ERR_CALIB_LOW == eventType || PTC_CB_EVENT_ERR_CALIB_HIGH == eventType) {
    playStarTrekChirp(); 
  }
}

void setup() {
  ptc_add_selfcap_node(&touch_node, 0, PIN_TO_PTC(TOUCH_PIN));

  uint32_t start_time = millis();
  while (millis() - start_time < 1000) {
      ptc_process(millis());
  }

#ifdef ENABLE_LOW_POWER
  // --- LOW POWER CONFIGURATION ---
  _PROTECTED_WRITE(BOD.CTRLA, BOD_SLEEP_DIS_gc);

  RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;
  while (RTC.STATUS > 0); 
  
  RTC.PITCTRLA = RTC_PERIOD_CYC4096_gc | RTC_PITEN_bm;
  
  EVSYS.ASYNCCH3 = EVSYS_ASYNCCH3_PIT_DIV4096_gc;
  EVSYS.ASYNCUSER1 = EVSYS_ASYNCUSER1_ASYNCCH3_gc;
  
  SLPCTRL.CTRLA = SLEEP_MODE_STANDBY | SLPCTRL_SEN_bm;
#endif

  pinMode(LED_PWR_EN, OUTPUT);
  setLedPower(false);
  pinMode(PIEZO_PIN, OUTPUT);
  digitalWrite(PIEZO_PIN, LOW);
}

void loop() {
  ptc_process(millis());    

#ifdef ENABLE_LOW_POWER
  // Low power sleep
  set_sleep_mode(SLEEP_MODE_STANDBY);
  sleep_enable();
  sleep_cpu();
  sleep_disable();
#else
  // When awake, add a tiny delay to prevent the loop from running 
  // unnecessarily fast and masking timing issues.
  delay(10);
#endif
}