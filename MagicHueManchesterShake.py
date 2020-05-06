import time
import magichue

FREQ = 15

T = 1/FREQ

HANDSHAKE_LEN = 10

RGB = (0, 10, 10)


light = magichue.Light('192.168.1.200', confirm_receive_on_send=False, allow_fading=False)

time.sleep(0.1)

if light.is_white:
    light.is_white = False

time.sleep(0.1)

if light.rgb != RGB:
    light.rgb = RGB

time.sleep(0.1)

#read message as binary and make an array of the bytes
f = open ("HiddenMessage.txt", "rb")
message = f.read()
b = bytearray(message)

#generate handshake at beginning of bits array
bits = [1,1,0,0]*(HANDSHAKE_LEN//2) + [1,0]
sum = 0

for byte in b:
    #add sum of bits in byte
    for i in range(8):
        sum += ((byte >> (7-i)) & 1)
    #if the sum is odd, make parity bit 1
    if sum % 2 == 0:
        byte |= 0b10000000
    #add every bit to the bits array
    for i in range(8):
        print((byte >> (7-i)) & 1)
        if (byte >> (7-i)) & 1 == 1:
            bits += [0, 1]
        else:
            bits += [1, 0]
    #add stop and start signals between every byte (16 transmitted bits)
    bits += [1,0]

#send message over bulb
for bit in bits:
    if bit == 1:
        light.on = True
    else:
        light.on = False
    time.sleep(T)

light.on = False