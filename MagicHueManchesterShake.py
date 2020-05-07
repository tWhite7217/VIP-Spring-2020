import time
import magichue

FREQ = 5

T = 1/FREQ

HANDSHAKE_LEN = 10
PARITY_LEN = 2      # number of bytes associated with a parity bit
PARITY_MODE = False  # True if parity is for every byte and is MSB
                    # False if parity is for PARITY_LEN bytes and is LSB

RGB = (0, 10, 10)

# change IP address for bulb
light = magichue.Light('192.168.1.227', confirm_receive_on_send=False, allow_fading=False)

initialState = light.on

time.sleep(0.1)

if light.is_white:
    light.is_white = False

time.sleep(0.1)

if light.rgb != RGB:
    light.rgb = RGB

time.sleep(0.1)

# read message as binary and make an array of the bytes
f = open("HiddenMessage.txt", "rb")
message = f.read()
b = bytearray(message)

if PARITY_MODE:
    # generate handshake at beginning of bits array
    if initialState:
        bits = [0, 0, 1, 1]*(HANDSHAKE_LEN//2 - 1) + [0, 0, 1, 0]
    else:
        bits = [1, 1, 0, 0]*(HANDSHAKE_LEN//2) + [1, 0]
    sum = 0

    for byte in b:
        # add sum of bits in byte
        byte &= 0b01111111
        for i in range(8):
            sum += ((byte >> (7-i)) & 1)
        #if the sum is even, make parity bit 1
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
        sum = 0

else:
    # generate handshake at beginning of bits array
    bits = [1, 1, 0, 0]*(HANDSHAKE_LEN//2) + [1, 0]
    sum = 0         # running sum of bits per parity set
    parity_count = 0    # the byte of data we are on in the parity set

    for byte in b:
        # increment parity_count, which tells us which byte of data we are on for the parity set
        parity_count = (parity_count + 1) % PARITY_LEN
        # if we are on the first byte in the parity set, we want to reset the sum to 0
        if parity_count == 1:
            sum = 0
        # add sum of bits in byte
        for i in range(8):
            sum += ((byte >> (7-i)) & 1)

        # add bits to the bits array which will used to send data over bulb
        # for each byte, we want to transmit the 7 least significant bits (don't include parity)
        for i in range(7):
            print((byte >> (6-i)) & 1)
            if (byte >> (6-i)) & 1 == 1:
                bits += [0, 1]
            else:
                bits += [1, 0]
        # if we are on the last byte in the parity set, we want to compute the parity bit at LSB
            # results in less total parity bits sent -> greater overall throughput
        if parity_count == 0:
            # Compute parity bit- if the sum is even, make the parity bit 1
            if sum % 2 == 0:
                print(1)
                bits += [0, 1]
            else:
                print(0)
                bits += [1, 0]
            # add stop and start signals between every PARITY_LEN bytes (PARITY_LEN*2 transmitted bits)
            bits += [1, 0]

# send message over bulb
for bit in bits:
    if bit == 1:
        light.on = True
    else:
        light.on = False
    time.sleep(T)

light.on = initialState