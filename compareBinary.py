f = open("ExpectedBinary.txt", "r")
expected = f.read()
expected = expected[::2]

f = open("ActualBinary.txt", "r")
actual = f.read()
actual = actual[::2]

exLength = len(expected)
print(exLength)
sum = 0

acLength = len(actual)

for i in range(min(exLength, acLength)):
    result = expected[i] == actual[i]
    print(expected[i] == actual[i])
    if result:
        sum += 1

print(sum/exLength)