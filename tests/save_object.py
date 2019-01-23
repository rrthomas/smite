# Test save_object().
#
# (c) Reuben Thomas 1995-2019
#
# The package is distributed under the GNU Public License version 3, or,
# at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
# RISK.

from smite import *
size = 256
VM = State(size)
VM.globalize(globals())


# Test data
M_word[0] = 0x01020304
M_word[word_size] = 0x05060708

# Test results
addr = [(size + 1) * word_size, 0, 0]
length = [16, 3000, 16]
correct = [-1, -1, 0]

# Test
def try_save(file, address, length):
    ret = save(file, address, length)
    print("save_object(\"{}\", {}, {}) returns {}".format(file, address, length, ret), end='')
    return ret

for i in range(3):
    res = try_save("saveobj", addr[i], length[i])
    if i != 2:
      os.remove("saveobj")
    print(" should be {}".format(correct[i]))
    if res != correct[i]:
        print("Error in save_object() test {}".format(i + 1))
        sys.exit(1)

ret = load("saveobj", 4 * word_size)
os.remove("saveobj")

for i in range(4):
    old = M_word[i * word_size]
    new = M_word[(i + 4) * word_size]
    print("Word {} of memory is {}; should be {}".format(i, new, old))
    if new != old:
        print("Error in save_object() tests: loaded file does not match data saved")
        sys.exit(1)

print("save_object() tests ran OK")