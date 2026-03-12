#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <ptc.h>

// Using confirmed PIN_Pxn naming from btbm.ch
#define PIEZO_PIN   PIN_PA1
#define TOUCH_PIN   PIN_PA4
#define LED_DATA    PIN_PB4
#define LED_PWR_EN  PIN_PA7
#define NUM_LEDS    4

Adafruit_NeoPixel strip(NUM_LEDS, LED_DATA, NEO_GRB + NEO_KHZ800);
uint32_t nextSleep = 10000;
// Global touch node handle
cap_sensor_t touch_node;   // as an example, could also be part of the array

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
// callbacks that are called by ptc_process at different points to ease user interaction
void ptc_event_cb_touch(const ptc_cb_event_t eventType, cap_sensor_t *node) {
  if (PTC_CB_EVENT_TOUCH_DETECT == eventType) {
    playStarTrekChirp();
  } 
}

void ptc_event_cb_calibration(const ptc_cb_event_t eventType, cap_sensor_t* node)  {
  if (PTC_CB_EVENT_ERR_CALIB_LOW == eventType || PTC_CB_EVENT_ERR_CALIB_HIGH == eventType) {
    playStarTrekChirp(); // Alert us if calibration fails
  }
}

void setup() {
  // Enable WDT for 2s wakeup as seen in btbm.ch code
  //_PROTECTED_WRITE(WDT.CTRLA, WDT_PERIOD_2KCLK_gc);
  //// Initialize the PTC hardware
  // Add PA4 as a self-capacitance sensor
  // Parameters: pin, gain, freq_hop, oversampling
  ptc_add_selfcap_node(&touch_node, 0, PIN_TO_PTC(TOUCH_PIN));
 // In low-power mode, the ADC is triggered based on positive Event fanks
  // Due to the big variety of possible Event sources, it is up to the
  // User to set up the Routing of the Event System.
  // This example will use the RTC PIT as Event generator, as it allows us
  // to use the low-power oscillator.
 // ptc_node_set_thresholds(&touch_node, int16_t th_in, int16_t th_out);
  // Enable PIT for the EVSYS with 32kHz / 4096 giving us about 8Hz
  
  uint32_t start_time = millis();
  while (millis() - start_time < 1000) {
      ptc_process(millis());
  }
  
  RTC.PITCTRLA = RTC_PITEN_bm;
  EVSYS.ASYNCCH3 = EVSYS_ASYNCCH3_PIT_DIV4096_gc;
  EVSYS.ASYNCUSER1 = EVSYS_ASYNCUSER1_ASYNCCH3_gc;
  // Stand-by sleep
  SLPCTRL.CTRLA = SLEEP_MODE_STANDBY | SLPCTRL_SEN_bm;
  
  pinMode(LED_PWR_EN, OUTPUT);
  setLedPower(false);
  pinMode(PIEZO_PIN, OUTPUT);
}

void loop() {
// Trigger a measurement cycle
  ptc_process(millis());    // main ptc task, requires regular calls


  // Check the touch status
  // ptc_is_node_touched returns true if the change exceeds the threshold
  //if (ptc_get_node_touched(touchNode)) {
  //  playStarTrekChirp();
  //  delay(500); 
 // }

  // Low power sleep
  set_sleep_mode(SLEEP_MODE_STANDBY);
  sleep_enable();
  sleep_cpu();
  sleep_disable();
  //delay(100);
}
