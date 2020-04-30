import time
import magichue

FREQ = 15       # Frequency

T = 1/FREQ      # Period
RGB = (0, 10, 10)

light = magichue.Light('192.168.1.200', confirm_receive_on_send=False, allow_fading=False)

time.sleep(0.1)

if light.is_white:
    light.is_white = False

time.sleep(0.1)

if light.rgb != RGB:
    light.rgb = RGB

time.sleep(0.1)

# Read file that is to be transferred over bulb
f = open ("HiddenMessage.txt", "rb")
message = f.read()
b = bytearray(message)


#Straight Binary Encoding: bit = 1 -> turn bulb on, bit = 0 -> turn bulb off
for byte in b:
    #print(hex(byte))
    for i in range(8):
        print((byte >> (7-i)) & 1)
        if (byte >> (7-i)) & 1 == 1:
            light.on = True
        else:
            light.on = False
        time.sleep(T)

light.on = False
