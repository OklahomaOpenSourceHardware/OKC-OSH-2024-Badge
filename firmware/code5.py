

#latest firmware
#https://github.com/OklahomaOpenSourceHardware/OKC-OSH-2024-Badge/tree/main-2/firmware

#Board files
#https://github.com/OklahomaOpenSourceHardware/OKC-OSH-2024-Badge/tree/main-2/B-Sides%202024
#import array
import time
import board
import random
from rainbowio import colorwheel
import neopixel
import asyncio
import digitalio
import pulseio
import adafruit_irremote


#system variables and constants

# packet format [wait_time_mS,inverse (wait_time_mS), colorwheel(0:255), inverse colorwheel(0:255)]
#each badge has a timeslot 0-255 with a time window of 108mS, and a full cycle of 27.54 seconds
#every approximately 0.5 minutes, the badge will self segrigate into an empty timeslot, and attempt to phase match the incoming pulses

#once locked, the badge will transmit it's chosen color
#the color is selectable by pressing buttons (BTN_0,BTN_1)

#as your badge recieves messages, the color will shift based on the colors of people around you



tornado_pixels = 40
SAO_2024_pixels = 4

#GPIO0 button 0
btn_0 = digitalio.DigitalInOut(board.GP0)
btn_0.direction = digitalio.Direction.INPUT
btn_0.pull = digitalio.Pull.DOWN
prev_state_0 = btn_0.value
#GPIO1 button 1
btn_1 = digitalio.DigitalInOut(board.GP1)
btn_1.direction = digitalio.Direction.INPUT
btn_1.pull = digitalio.Pull.DOWN
prev_state_1 = btn_1.value
#GPIO2 SDA_QUICC
#GPIO3 SCL_QUICC

#GPIO4 USBBOOT

#GPIO5 SAO_EN(False), pull down to disable SAO power

#GPIO6 SAO_TX_45
pixels_SAO_45 = neopixel.NeoPixel(board.GP6, SAO_2024_pixels, brightness=0.05, auto_write=False)
#GPIO7 SAO_RX_45
#GPIO8 SAO_SCL
#GPIO9 SAO_SDA
#GPIO10 DIO_LV
pixels_DIO_Tornado = neopixel.NeoPixel(board.GP10, tornado_pixels, brightness=0.05, auto_write=False)

#GPIO13 LED_RED, simple led, set pin value high/True to light the led
pin_led = digitalio.DigitalInOut(board.GP13)
pin_led.direction = digitalio.Direction.OUTPUT

#GPIO19 SAO_TX_H
pixels_SAO_H = neopixel.NeoPixel(board.GP19, SAO_2024_pixels, brightness=0.05, auto_write=False)
#GPIO18 SAO_RX_H

#GPIO23 (False), negative logic, has external pull up resistor
pulsein = pulseio.PulseIn(board.GP23, maxlen=200, idle_state=True)

#GPIO24 IR_led_driver(True) positive logic, drive high to turn on IR LEDs
pulseout = pulseio.PulseOut(board.GP24, frequency=38000, duty_cycle=2 ** 15)

#GPIO25 LED_EN(True), this pin enabled the fuse/switch  for the SAO and neopixel leds
pin = digitalio.DigitalInOut(board.GP25)
pin.direction = digitalio.Direction.OUTPUT

#GPIO28 PWR_FLT, pulled low when fault is present on the LED lines
pin_pwr = digitalio.DigitalInOut(board.GP28)
pin_pwr.switch_to_input()
pin_pwr.direction = digitalio.Direction.INPUT


encoder = adafruit_irremote.GenericTransmit(header=[9582, 4529], one=[559, 559], zero=[559, 1699], trail=105)
decoder = adafruit_irremote.GenericDecode()
pulsein.clear()
pulsein.resume() #trigger_microseconds
nonblocking_decoder = adafruit_irremote.NonblockingGenericDecode(pulsein)

t0 = next_heartbeat = time.monotonic()


RED = (255, 0, 0)
YELLOW = (255, 150, 0)
GREEN = (0, 255, 0)
CYAN = (0, 255, 255)
BLUE = (0, 0, 255)
PURPLE = (180, 0, 255)
pin.value = True
color_choice = (random.randint(0, 255))

trigger_microseconds = 0.108
time_slot = random.randint(0, 255)
free_time_slots = [True] * 255
free_time_slots[time_slot] = False
next_time_slot = time_slot
next_transmission_microseconds = trigger_microseconds * 255 + t0
time_offset = 0

async def find_next_occurrence(array, value):
  start_index = 0
  while start_index < len(array):
    index = array.index(value, start_index)
    if index != -1:
      return index
    start_index += 1
  return -1

async def color_chase(pixels,num_pixels, color, wait):
    for i in range(num_pixels):
        pixels[i] = color
        await asyncio.sleep(wait)
        pixels.show()
    #time.sleep(0.1)


async def rainbow_cycle(pixels,num_pixels,wait):
    for j in range(255):
        for i in range(num_pixels):
            rc_index = (i * 256 // num_pixels) + j
            pixels[i] = colorwheel(rc_index & 255)
        pixels.show()
        await asyncio.sleep(wait)

async def fuzzy_pulse_compare(pulse1, pulse2, fuzzyness=0.2):
    if len(pulse1) != len(pulse2):
        return False
    for i in range(len(pulse1)):
        threshold = int(pulse1[i] * fuzzyness)
        if abs(pulse1[i] - pulse2[i]) > threshold:
            return False
    return True

async def bit_invert(n: int) -> int:
    if n < 0:
        # Convert negative integer to its equivalent positive value
        unsigned_n = 256 + n  # Assuming 8-bit integers
    else:
        unsigned_n = n
    return (unsigned_n ^ 0xFF) & 0xFF


async def main():
    received_code = [time_slot, bit_invert(time_slot), color_choice, bit_invert(color_choice)] #[random.randint(0, 255),random.randint(0, 255),random.randint(0, 255),random.randint(0, 255)] #[255, 5, 255, 5]
    print(received_code)
    transmitted_code = received_code

    while True:
           
        t_now = time.monotonic()
        if  btn_0.value:
            print("BTN_0 is down")
            color_choice -=1
        if  btn_1.value:
            print("BTN_1 is down")
            color_choice +=1        

        if t_now > next_transmission_microseconds:
            print("transmission slot: ", next_time_slot)
            if next_time_slot == time_slot:
                encoder.transmit(pulseout, transmitted_code)
        #    if (len(nonblocking_decoder.read()) >= 1) and free_time_slots[next_time]:
            next_time_slot += 1
            if next_time_slot > 255:
                next_time_slot = 0
            next_transmission_microseconds = trigger_microseconds + t_now + time_offset
            
            
            
            for message in nonblocking_decoder.read():
                print(f"t={time.monotonic() - t0:.3} New Message")
                print("Heard", len(message.pulses), "Pulses:", message.pulses)
                print("statistics", (adafruit_irremote.bin_data(message.pulses)))
                if isinstance(message, adafruit_irremote.IRMessage):
                    print("Decoded:", message.code)
                    try:
                        received_code = decoder.decode_bits(message.code)
                        print("Decoded:", received_code)
                        if(received_code[0] == bit_invert(received_code[1]) and received_code[2] == bit_invert(received_code[3])):
                            free_time_slots[received_code[0]] = False
                            
                            color_chase(pixels_DIO_Tornado,tornado_pixels,colorwheel(received_code[2]), 0.0)
                            color_choice = (color_choice+received_code[2])/2
                            pin_led.value = True
                            print("success")
                        else:
                            pin_led.value = False
                            print("failed")
                        for x , y in zip(received_code , transmitted_code ):
                            y = (x+y)/2
                
                        print(received_code[1:4])
                        print(transmitted_code)
            
                    except adafruit_irremote.FailedToDecode:
                        print("Failed to decode")
                        trigger_microseconds += 1
                    except adafruit_irremote.IRNECRepeatException:  # unusual short code!
                        print("NEC repeat!")
                    except adafruit_irremote.IRDecodeException as e:     # failed to decode
                        print("Failed to decode: ", e.args)
                
                elif isinstance(message, adafruit_irremote.NECRepeatIRMessage):
                    print("NEC repeat!")
                elif isinstance(message, adafruit_irremote.UnparseableIRMessage):
                    print("Failed to decode", message.reason)
                print("----------------------------")
           
        # This heartbeat confirms that we are not blocked somewhere above.
        t = time.monotonic()
        if t > next_heartbeat:
            print(f"t={time.monotonic() - t0:.3} Heartbeat")
            next_heartbeat = t + 0.1

      
        #print((color_choice))
        color_chase(pixels_DIO_Tornado,tornado_pixels,colorwheel(color_choice), 0.0)
        color_chase(pixels_DIO_Tornado,tornado_pixels,colorwheel(color_choice+6), 0.0)
        rainbow_cycle(pixels_SAO_H,SAO_2024_pixels,0)  # Increase the number to slow down the rainbow
        rainbow_cycle(pixels_SAO_45,SAO_2024_pixels,0)  # Increase the number to slow down the rainbow
        # Clear the rest
        #pulsein.clear()
        # Resume with an 80 microsecond active pulse
        #pulsein.resume() #trigger_microseconds



    







