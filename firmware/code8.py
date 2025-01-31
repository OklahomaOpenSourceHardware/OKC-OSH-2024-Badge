

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
import keypad



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
# btn_0 = digitalio.DigitalInOut(board.GP0)
# btn_0.direction = digitalio.Direction.INPUT
# btn_0.pull = digitalio.Pull.DOWN
# prev_state_0 = btn_0.value
#GPIO1 button 1
# btn_1 = digitalio.DigitalInOut(board.GP1)
# btn_1.direction = digitalio.Direction.INPUT
# btn_1.pull = digitalio.Pull.DOWN
# prev_state_1 = btn_1.value
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
COLOR_LIST = [RED,YELLOW,GREEN,CYAN,BLUE,PURPLE]
pin.value = True


TRIGGER_MICROSECONDS = 0.108


time_offset = 0

def bit_invert(n: int) -> int:
    if n < 0:
        # Convert negative integer to its equivalent positive value
        unsigned_n = 256 + n  # Assuming 8-bit integers
    else:
        unsigned_n = n
    return (unsigned_n ^ 0xFF) & 0xFF


class ButtonState:
    def __init__(self, initial_state):
        self.state=initial_state
        
class Interval:
    # pylint: disable=too-few-public-methods
    def __init__(self, initial_value):
        self.value = initial_value

class LED_State:
     # pylint: disable=too-few-public-methods
    def __init__(self, initial_value):
        self.value = initial_value



class Transmission_State:
    def __init__(self, initial_state, initial_time):
        self.state =initial_state
        self.next_transmission_microseconds = TRIGGER_MICROSECONDS * 256 #+ initial_time
        self.time_slot = random.randint(0, 255)
        self.free_time_slots = [True] * 256
        self.free_time_slots[self.time_slot] = False
        self.next_time_slot = self.time_slot
        self.color_choice = (random.randint(0, 255))
        self.received_code = [self.time_slot, bit_invert(self.time_slot), self.color_choice, bit_invert(self.color_choice)] #[random.randint(0, 255),random.randint(0, 255),random.randint(0, 255),random.randint(0, 255)] #[255, 5, 255, 5]
        self.transmitted_code = self.received_code
        

def evaluate_transmission(transmission_state):
    t_now = time.monotonic()
    if t_now > transmission_state.next_transmission_microseconds:
        transmission_state.next_transmission_microseconds = TRIGGER_MICROSECONDS + t_now + time_offset
        print("timeslot adjustment")

async def recieve_transmission(transmission_state): 
    while True:
        for message in nonblocking_decoder.read():
            print(f"t={time.monotonic() - t0:.3} New Message")
            print("Heard", len(message.pulses), "Pulses:", message.pulses)
            print("statistics", (adafruit_irremote.bin_data(message.pulses)))
            if isinstance(message, adafruit_irremote.IRMessage):
                print("Decoded:", message.code)
                try:
                    transmission_state.received_code = decoder.decode_bits(message.code)
                    print("Decoded:", transmission_state.received_code)
                    if(transmission_state.received_code[0] == bit_invert(transmission_state.received_code[1]) and transmission_state.received_code[2] == bit_invert(transmission_state.received_code[3])):
                        transmission_state.free_time_slots[transmission_state.received_code[0]] = False
                        
                        #color_chase(pixels_DIO_Tornado,tornado_pixels,colorwheel(transmission_state.received_code[2]), 0.0)
                        transmission_state.color_choice = (transmission_state.color_choice+transmission_state.received_code[2])/2
                        pin_led.value = True
                        print("success")
                    else:
                        pin_led.value = False
                        print("failed")
                    for x , y in zip(transmission_state.received_code , transmission_state.transmitted_code ):
                        y = (x+y)/2
            
                    print(transmission_state.received_code[1:4])
                    print(transmission_state.transmitted_code)
        
                except adafruit_irremote.FailedToDecode:
                    print("Failed to decode1")
                    time_offset += 1
                except adafruit_irremote.IRNECRepeatException:  # unusual short code!
                    print("NEC repeat!")
                except adafruit_irremote.IRDecodeException as e:     # failed to decode
                    print("Failed to decode2: ", e.args)
            
            elif isinstance(message, adafruit_irremote.NECRepeatIRMessage):
                print("NEC repeat!")
            elif isinstance(message, adafruit_irremote.UnparseableIRMessage):
                print("Failed to decode3", message.reason)
            print("----------------------------")
            await asyncio.sleep(0)
        await asyncio.sleep(0)

async def evaluate_timeslot(transmission_state):
    while True:
        t_now = time.monotonic()       
        next_transmission_microseconds = TRIGGER_MICROSECONDS + t_now + time_offset
        #print("transmission slot:next time slot ", transmission_state.next_time_slot, ":", transmission_state.time_slot)
        #print("time_now:", t_now, "next_transmission:", next_transmission_microseconds)
        if transmission_state.next_time_slot == transmission_state.time_slot:
            encoder.transmit(pulseout, transmission_state.transmitted_code)
        #    if (len(nonblocking_decoder.read()) >= 1) and free_time_slots[next_time]:
        transmission_state.next_time_slot += 1
        if transmission_state.next_time_slot > 255:
            transmission_state.next_time_slot = 0
        await asyncio.sleep(transmission_state.next_transmission_microseconds/100)

def find_next_occurrence(array, value):
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
        pixels.show()
        await asyncio.sleep(wait)        
    await asyncio.sleep(0.1)

async def color_cycle(transmission_state):
    while True:
        await color_chase(pixels_DIO_Tornado,tornado_pixels,colorwheel(transmission_state.color_choice), 0.1)   
        await asyncio.sleep(0.1)


async def rainbow_cycle(pixels,num_pixels,wait):
    while True:
        for j in range(255):
            for i in range(num_pixels):
                rc_index = (i * 256 // num_pixels) + j
                pixels[i] = colorwheel(rc_index & 255)
            pixels.show()
            await asyncio.sleep(wait)

def fuzzy_pulse_compare(pulse1, pulse2, fuzzyness=0.2):
    if len(pulse1) != len(pulse2):
        return False
    for i in range(len(pulse1)):
        threshold = int(pulse1[i] * fuzzyness)
        if abs(pulse1[i] - pulse2[i]) > threshold:
            return False
    return True


async def monitor_buttons(transmission_state):
    with keypad.Keys(
        (board.GP0, board.GP1), value_when_pressed=True, pull=False
    ) as keys:
        while True:
            key_event = keys.events.get()
            if key_event and key_event.pressed:
                if key_event.key_number == 0:
                    print("BTN_0 is down")
                    
                    if transmission_state.color_choice > 255:
                        transmission_state.color_choice = 0
                    else:
                        transmission_state.color_choice +=10
                else:
                    print("BTN_1 is down")
                    
                    if transmission_state.color_choice == 0:
                        transmission_state.color_choice = 255
                    else:
                        transmission_state.color_choice -=10
            await asyncio.sleep(0)
async def blink(pin, interval):
    """Blink the given pin forever.
    The blinking rate is controlled by the supplied Interval object.
    """
    with digitalio.DigitalInOut(pin) as led:
        led.switch_to_output()
        while True:
            led.value = not led.value
            await asyncio.sleep(interval)
            

async def status_text(transmission_state):
    while True:
        #$print(transmission_state.initial_state)
        print("next_transmission_microseconds:",transmission_state.next_transmission_microseconds)
        print("time_slot",transmission_state.time_slot)
      #  print("".join(map(str,[bin(i) for i in transmission_state.free_time_slots])))
        #print(sum(bit << idx, for idx, bit in enumerate(transmission_state.free_time_slots[::-1])))
        print("next time_slot",transmission_state.next_time_slot)
        print("color choice",transmission_state.color_choice)
        print("recieved code",transmission_state.received_code)
        print("transmitted code",transmission_state.transmitted_code)
        await asyncio.sleep(10)
async def main():
    t = time.monotonic()
    transmission_state = Transmission_State(False,t)
    while True:
 
        # This heartbeat confirms that we are not blocked somewhere above.
        #t = time.monotonic()
        #if t > next_heartbeat:
        #    print(f"t={time.monotonic() - t0:.3} Heartbeat")
        #    next_heartbeat = t + 0.1
        transmission_task = evaluate_transmission(transmission_state)
        timeslot_task = evaluate_timeslot(transmission_state)
        recieve_task  = recieve_transmission(transmission_state)
        #blink_task = asyncio.create_task(blink(board.GP13, 0.2))
        #print((color_choice))
        #color_chase(pixels_DIO_Tornado,tornado_pixels,colorwheel(color_choice), 0.0)
        status_task = status_text(transmission_state)
        button_task = monitor_buttons(transmission_state)
        tornado_task = color_cycle(transmission_state) #rainbow_cycle(pixels_DIO_Tornado,tornado_pixels,0)#
        sao_h_task = rainbow_cycle(pixels_SAO_H,SAO_2024_pixels,0)  # Increase the number to slow down the rainbow
        sao_45_task = rainbow_cycle(pixels_SAO_45,SAO_2024_pixels,0)  # Increase the number to slow down the rainbow
        # This will run forever, because neither task ever exits.
        await asyncio.gather(tornado_task, sao_h_task,sao_45_task,button_task,timeslot_task,recieve_task,status_task)#,blink_task)#,recieve_task,timeslot_task,transmission_task)

        # Clear the rest
        #pulsein.clear()
        # Resume with an 80 microsecond active pulse
        #pulsein.resume() #trigger_microseconds


asyncio.run(main())



