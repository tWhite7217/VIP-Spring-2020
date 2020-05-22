from termcolor import colored

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
correctSum = 0
bitSum = 0
totalCorrect = 0
detected = 0

acLength = len(actual)

dataBits = []

#for loop only as long as longest file to avoid out of bounds errors
for i in range(min(exLength, acLength)):
    # check if bits are the same
    result = expected[i] == actual[i]
    # sum the correct bits in an array
    if result:
        correctSum += 1
    # sum the bits to check the parity
    bitSum += int(actual[i])
    # every 4 bits:
    if (i+1) % 4 == 0:
        # should equal 1
        bitSet = correctSum/4
        # because of odd parity bit, if sum is even, an error has been detected
        errorDetected = bitSum % 2 == 0
        if errorDetected:
            detected += 1
        correctSum = 0
        bitSum = 0
        print ('bit set ' + str(i//4) + ': ' + str(bitSet == 1) + ' - ' + str(bitSet) + ' - error detected: ' + str(errorDetected))
        if bitSet == 1:
            totalCorrect += 1
    #when not a parity bit, add to dataBits
    else:
        dataBits += [int(actual[i])]

print('detected: ' + str(detected))
print('incorrect: ' + str((exLength//4) - totalCorrect))
print('bit sets correct: ' + str(totalCorrect/(exLength/4)))

byte = 0
letters = []

for i in range(len(dataBits)):
    byte |= dataBits[i] << (6-(i % 7))
    if (i + 1) % 7 == 0:
        letters += [byte]
        byte = 0

message = ""
lettersCorrect = 0

#read expected binary
f = open("HiddenMessage.txt", "r")
expectedText = f.read()

for letter in letters:
    message += chr(letter)

message = message.replace('\r', '')

wrongLetterIndexes = []

for i in range(min(len(message), len(expectedText))):
    if message[i] == expectedText[i]:
        lettersCorrect += 1
    else:
        wrongLetterIndexes += [i]

for i in wrongLetterIndexes:
    message = message[:i] + colored(message[i], 'red') + message[i+1:]
        

print(message)
print(expectedText)
print("Letters correct: " + str(lettersCorrect))