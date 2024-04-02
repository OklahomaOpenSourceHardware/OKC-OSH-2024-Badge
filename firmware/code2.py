import time
import board
import asyncio
from rainbowio import colorwheel
import neopixel
import digitalio
import pulseio
import adafruit_irremote


num_pixels = 4


pin = digitalio.DigitalInOut(board.GP25)
pin.direction = digitalio.Direction.OUTPUT
pin_led = digitalio.DigitalInOut(board.GP13)
pin_led.direction = digitalio.Direction.OUTPUT
pin_pwr = digitalio.DigitalInOut(board.GP28)
pin_pwr.switch_to_input()
pin_pwr.direction = digitalio.Direction.INPUT

pulsein = pulseio.PulseIn(board.GP23, maxlen=200, idle_state=True)
pulseout = pulseio.PulseOut(board.GP24, frequency=38000, duty_cycle=2 ** 15)
encoder = adafruit_irremote.GenericTransmit(header=[9582, 4529], one=[559, 559], zero=[559, 1699], trail=0)
decoder = adafruit_irremote.GenericDecode()
pulsein.clear()
pulsein.resume()



pixels = neopixel.NeoPixel(board.GP26, num_pixels, brightness=0.01, auto_write=False)


async def color_chase(color, wait):
    for i in range(num_pixels):
        pixels[i] = color
        await asyncio.sleep(wait)
        pixels.show()
    await asyncio.sleep(0.1)


def rainbow_cycle(wait):
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




RED = (255, 0, 0)
YELLOW = (255, 150, 0)
GREEN = (0, 255, 0)
CYAN = (0, 255, 255)
BLUE = (0, 0, 255)
PURPLE = (180, 0, 255)
received_code = [255, 5, 255, 5]
transmitted_code = [255, 5, 255, 5]
while True:
    pin.value = True
   
    #pulses_in = adafruit_irremote.NonblockingGenericDecoder(pulsein)
    #decoder = pulseio.NonblockingGenericDecoder(pulsein)
    
   # Pause while we do something with the pulses
    
    puslein_temp = decoder.read_pulses(pulsein)
    pulsein.pause()
  
    print(adafruit_irremote.bin_data(puslein_temp))
    print(puslein_temp)
   # time.sleep(1)
    try:
        received_code = decoder.decode_bits(puslein_temp)
        for x , y in zip(received_code , transmitted_code ):
            y = (x+y)/2
        
        print(received_code[1:4])
        print(transmitted_code)
       # for message in decoder.read():
       #     if isinstance(message, IRMessage):
       #         message.code 
       #     else:
       #         #pulses = decoder.read_pulses(pulses_in)
       #         received_code = decoder.decode_bits(message.code)
    except adafruit_irremote.FailedToDecode:
        print("Failed to decode")
    except adafruit_irremote.IRNECRepeatException:  # unusual short code!
        print("NEC repeat!")
    except adafruit_irremote.IRDecodeException as e:     # failed to decode
        print("Failed to decode: ", e.args)
        
        
    encoder.transmit(pulseout, transmitted_code)    
    print("Decoded:", received_code)
    if received_code == (255, 5, 255, 5):
        pin_led.value = True
        print("success")
    else:
        pin_led.value = False
        print("failed")
  
    color_chase(received_code[1:4], 0.1)
    # Clear the rest
    pulsein.clear()
    # Resume with an 80 microsecond active pulse
    pulsein.resume(80)
    
    if pin_pwr.value == True :
        pin_led.value = False
    if pin_pwr.value == False :
        pin_led.value = True
    
   # pixels.fill(RED)
   # pixels.show()
    # Increase or decrease to change the speed of the solid color change.
   # time.sleep(1)
   # pixels.fill(GREEN)
   # pixels.show()
   # time.sleep(1)
   # pixels.fill(BLUE)
   # pixels.show()
   # time.sleep(1)

   # color_chase(RED, 0.1)  # Increase the number to slow down the color chase
   # color_chase(YELLOW, 0.1)
   # color_chase(GREEN, 0.1)
   # color_chase(CYAN, 0.1)
   # color_chase(BLUE, 0.1)
   # color_chase(PURPLE, 0.1)

    #rainbow_cycle(0)  # Increase the number to slow down the rainbow
