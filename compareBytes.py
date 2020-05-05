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
totalCorrect = 0
detected = 0

acLength = len(actual)

#for loop only as long as longest file to avoid out of bounds errors
for i in range(min(exLength, acLength)):
    #check if bits are the same
    result = expected[i] == actual[i]
    #sum the correct bits in an array
    if result:
        sum += 1
    #every 8 bits:
    if (i+1) % 8 == 0:
        #should equal 1
        byte = sum/8
        #beacause of parity bit, if sum is odd, an error is detected
        errorDetected = sum % 2 == 0
        if errorDetected:
            detected += 1
        sum = 0
        print ('byte ' + str(i//8) + ': ' + str(byte == 1) + ' - ' + str(byte) + ' - error detected: ' + str(errorDetected))
        if byte == 1:
            totalCorrect += 1


print('detected: ' + str(detected))
print('incorrect: ' + str((exLength/8) - totalCorrect))
print('bytes correct: ' + str(totalCorrect/(exLength/8)))