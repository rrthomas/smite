# Test LIT.
#
# (c) Reuben Thomas 1994-2018
#
# The package is distributed under the MIT/X11 License.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
# RISK.

from smite import *
VM = State()
VM.globalize(globals())


# Test results
correct = [
    -257, 12345678, 4, -1 << (word_bit - 1),
    1 << (word_bit - 2), -1 << (word_bit - byte_bit)
]

encodings = []
if word_size == 4:
    encodings = [
        "\xff\xfe\xff\xff",
        "\x4e\x61\xbc\x00",
        "\x04\x00\x00\x00",
        "\x00\x00\x00\x80",
        "\x00\x00\x00\x40",
        "\x00\x00\x00\xff",
    ]
elif word_size == 8:
    encodings = [
        "\xff\xfe\xff\xff\xff\xff\xff\xff",
        "\x4e\x61\xbc\x00\x00\x00\x00\x00",
        "\x04\x00\x00\x00\x00\x00\x00\x00",
        "\x00\x00\x00\x00\x00\x00\x00\x80",
        "\x00\x00\x00\x00\x00\x00\x00\x40",
        "\x00\x00\x00\x00\x00\x00\x00\xff",
    ]
else:
    raise Exception("WORD_SIZE is not 4 or 8!")

assert(len(correct) == len(encodings))


# Test
def number_test(n, encoding):
    start = VM.here + 1
    print("here = {}".format(start))
    lit(n)
    length = VM.here - start

    bytes_ok = 0
    print("{} ({:#x}) encoded as: ".format(n, n), end='')
    for i in range(length):
        print("{:#02x} ".format(M[start + i]), end='')
        if ord(encoding[i]) == M[start + i]:
            bytes_ok += 1
    print()

    if bytes_ok != len(encoding):
        print("Error in numbers tests: encoding should be ", end='')
        for i in range(len(encoding)):
            print("{:#02x} ".format(ord(encoding[i])), end='')
        print()
        sys.exit(1)

for i in range(len(correct)):
    number_test(correct[i], encodings[i])

print("here = {}".format(VM.here))

step() # Load first number
for i in range(len(correct)):
    print("Data stack: {}".format(S))
    print("Correct stack: {} ({:#x})\n".format(correct[i], correct[i]))
    if S.depth.get() != 1 or correct[i] != S.pop():
        print("Error in numbers tests: PC = {:#x}".format(PC.get()))
        sys.exit(1)
    _, inst = disassemble_instruction(PC.get())
    print("I = {}".format(inst))
    step()

print("Numbers tests ran OK")