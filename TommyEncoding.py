import time
import magichue

FREQ = 15

T = 1/FREQ

HANDSHAKE_LEN = 10

RGB = (0, 10, 10)

# change IP address for bulb
light = magichue.Light('192.168.1.200', confirm_receive_on_send=False, allow_fading=False)

initialState = light.on

time.sleep(0.1)

if light.is_white:
    light.is_white = False

time.sleep(0.1)

if light.rgb != RGB:
    light.rgb = RGB

time.sleep(0.1)

# generate handshake at beginning of bits array
if initialState:
    bits = [0, 0, 1, 1]*(HANDSHAKE_LEN//2 - 1) + [0, 0, 1, 0]
else:
    bits = [1, 1, 0, 0]*(HANDSHAKE_LEN//2) + [1, 0]
sum = 0

# read message as binary and make an array of the bytes
f = open("HiddenMessage.txt", "rb")
message = f.read()
b = bytearray(message)

dataBits = []

#remove ASCII parity bit to obtain all data bits
for byte in b:
    print(hex(byte))
    for i in range(7):
        dataBits += [(byte >> (6-i)) & 1]

bitSum = 0

for i in range(len(dataBits)):
    #add dataBit to bits arrray
    print(dataBits[i])
    bits += [dataBits[i]]
    #sum every 3 bits
    bitSum += dataBits[i]
    #every 3 bits determine parity bit
    if (i + 1) % 3 == 0:
        #odd parity -> add 1 bit if sum even
        if bitSum % 2 == 0:
            # print(1)
            bits += [1]
        #odd parity -> add 0 bit if sum odd
        else:
            # print(0)
            bits += [0]
        bitSum = 0
    #every 12 bits add stop and start bits
    if (i +1) % 12 == 0:
        bits += [1,0]

# send message over bulb
for bit in bits:
    if bit == 1:
        light.on = True
    else:
        light.on = False
    time.sleep(T)

light.on = initialState