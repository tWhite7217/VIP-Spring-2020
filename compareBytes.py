PARITY_LEN = 2      # number of bytes associated with a parity bit
PARITY_MODE = False  # True if parity is for every byte and is MSB
                    # False if parity is for PARITY_LEN bytes and is LSB
print('PARITY MODE: '+ str(PARITY_MODE))
#read expected binary
f = open("ExpectedBinary.txt", "r")
expected = f.read()
#remove newline characters
expected = expected[::2]

#read actual binary
f = open("ActualBinary.txt", "r")
actual = f.read()
#remove newline characters
actual = actual[::2]

exLength = len(expected)
print(exLength)
sum = 0
bitSum = 0
totalCorrect = 0
detected = 0

acLength = len(actual)

#for loop only as long as longest file to avoid out of bounds errors
for i in range(min(exLength, acLength)):
    # check if bits are the same
    result = expected[i] == actual[i]
    # sum the correct bits in an array
    if result:
        sum += 1
    # sum the bits to check the parity
    bitSum += int(actual[i])
    if PARITY_MODE:
        # every 8 bits:
        if (i+1) % 8 == 0:
            # should equal 1
            byte = sum/8
            # beacause of parity bit, if sum is even, an error is detected
            errorDetected = bitSum % 2 == 0
            if errorDetected:
                detected += 1
            sum = 0
            bitSum = 0
            print ('byte ' + str(i//8) + ': ' + str(byte == 1) + ' - ' + str(byte) + ' - error detected: ' + str(errorDetected))
            if byte == 1:
                totalCorrect += 1
    else:
        # every PARITY_LEN*7 +1 bits:
        if (i + 1) % (PARITY_LEN*7 + 1) == 0:
            # set variable should equal 1, meaning all results in the set are correct
            set = sum/(PARITY_LEN*7 + 1)
            # beacause of parity bit, if sum is even, an error is detected
            errorDetected = bitSum % 2 == 0
            if errorDetected:
                detected += 1
            sum = 0
            bitSum = 0
            print('set ' + str(i // (PARITY_LEN*7 + 1) ) + ': ' + str(set == 1) + ' - ' + str(set) + ' - error detected: ' + str(
                errorDetected))
            if set == 1:
                totalCorrect += 1

print('detected: ' + str(detected))
print('incorrect: ' + str((exLength/8) - totalCorrect))
if PARITY_MODE:
    print('bytes correct: ' + str(totalCorrect/(exLength/8)))
else:
    print('sets correct: ' + str(totalCorrect / (exLength / (PARITY_LEN*7 + 1))))