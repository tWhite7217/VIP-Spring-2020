import time
import magichue

FREQ = 5

T = 1/FREQ

RGB = (0, 10, 10)

light = magichue.Light('192.168.1.200', confirm_receive_on_send=False, allow_fading=False)

time.sleep(0.1)

if light.is_white:
    light.is_white = False

time.sleep(0.1)

if light.rgb != RGB:
    light.rgb = RGB

time.sleep(0.1)

f = open ("HiddenMessage.txt", "rb")
message = f.read()
b = bytearray(message)
manchester = [1,1,0,0,1,1,0,0,1,1]

for byte in b:
    for i in range(8):
        bit = (byte >> (7-i)) & 1
        print(bit)
        if bit == 1:
            manchester += [0, 1]
        else:
            manchester += [1, 0]

for bit in manchester:
    if bit == 1:
        light.on = True
    else:
        light.on = False
    time.sleep(T)

light.on = False