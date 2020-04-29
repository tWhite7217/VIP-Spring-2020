import time
import magichue

FREQ = 15       # Frequency

T = 1/FREQ      # Period
t = T/2             # half the period
CLK = False         # CLK source
manchester = False  # manchester encoding value for bulb
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

# Move message contents into array of bits
bitStream= [False for x in range(len(b)*8)]
byteCount = 0
for byte in b:
    for i in range(8):
        if (byte >> (7 - i)) & 1 == 1:
            bitStream[byteCount*8 + i] = True
        else:
            bitStream[byteCount * 8 + i] = False
    byteCount = byteCount +1

""" #Straight Binary Encoding: bit = 1 -> turn bulb on, bit = 0 -> turn bulb off
for byte in b:
    #print(hex(byte))
    for i in range(8):
        print((byte >> (7-i)) & 1)
        if (byte >> (7-i)) & 1 == 1:
            light.on = True
        else:
            light.on = False
        time.sleep(T)
"""

# Use Manchester Encoding to send message to bulb
    # Logic 0: 1 to 0 downward transition at bit center
    # Logic 1: 0 to 1 upward transition at bit center
    # Problem: Handshaking with the sensor
byteCount = 0       # keeps track of what byte of data are we on
bitCount = 0        # keeps track of what bit of the byte are we on (0-7)
clkCount = 0        # clkCount%2 = 0 means we are on a new period and can move to next bit
print("Start of Message\n")
while True: # each time through loop is half a period
    CLK = not CLK # alternating clock
    bit = bitStream[byteCount * 8 + bitCount] # find value of bit we are sending
    if CLK: # when clock is high, we must set up our transition
        if bit == 1:
            manchester = False
        else:
            manchester = True
            if(manchester):
            print(1)
        else:
            print(0)
    else: # when clock goes low, we perform transition
        manchester = not manchester
        print(manchester)
    light.on = manchester
    time.sleep(t)
    clkCount = clkCount +1
    if clkCount%2 == 0: # new period -> move to next bit in bitStream
        bitCount = bitCount + 1
        if bitCount == 8: # we have reached the end of a byte
            bitCount = 0
            byteCount = byteCount+1
        if byteCount == len(b): # we have reached the end of the message
            bitCount = 0
            byteCount = 0
            print("End of Message\n")
            print("Start of Message\n")

light.on = False
