'''
Mit assembler.

(c) Mit authors 2019-2020

The package is distributed under the MIT/X11 License.

THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
RISK.
'''

from .binding import is_aligned, sign_extend, uword_max, word_bit, word_bytes
from .enums import TERMINAL_OPCODES
from .enums import Instructions as I


class Assembler:
    '''
    Represents the state of an assembler.

    Public fields:
     - pc - the value of the simulated pc register.
     - ir_addr - the address of the current opcode word, i.e. the word from
       which the simulated `ir` register was loaded, if any. Otherwise, i.e.
       after a terminal instruction or in the initial state, `ir_addr == pc`.
     - ir_shift - the number of opcode bits already in the word at `ir_addr`.
    '''
    def __init__(self, state, pc=None):
        '''
        `pc` defaults to the current pc value of `state`.
        '''
        self.state = state
        self.pc = pc
        if pc is None:
            self.pc = self.state.pc
        assert is_aligned(self.pc)
        self.label()

    def label(self):
        '''
        Skips to the start of a word, and returns its address.
        '''
        self.ir_addr = self.pc
        self.ir_shift = 0
        return self.pc

    def goto(self, pc):
        '''
        Set the assembly pointer to the given address.
        '''
        assert is_aligned(pc)
        self.pc = pc
        self.label()

    def word(self, value):
        '''
        Writes a word with value `value` at `pc` and increments `pc`.
        '''
        self.state.M_word[self.pc] = value
        self.pc += word_bytes

    def _fetch(self):
        '''
        Start a new word if we need to.
        '''
        if self.ir_addr == self.pc:
            assert self.ir_shift == 0
            self.word(0)

    def bytes(self, bytes):
        '''
        Writes `bytes` at `pc`, followed by padding to the next word.
        '''
        assert self.ir_shift == 0
        for b in bytes:
            self.state.M[self.pc] = b
            self.pc += 1
        # Align `pc`
        self.pc = ((self.pc - 1) & (word_bytes - 1)) + word_bytes

    def fit(self, opcode, operand=None):
        '''
        Determine whether the given extended opcode fits in the current word.
        If so, return the updated word; otherwise, None.
        '''
        assert self.ir_addr != self.pc, "Forgot to call `_fetch()`"

        opcode = int(opcode)
        if operand is not None:
            operand = int(operand)

        ir = self.state.M_word[self.ir_addr]
        ir |= opcode << self.ir_shift
        if opcode in TERMINAL_OPCODES:
            if operand is None:
                operand = 0 if opcode & 0x80 == 0 else -1
            ir |= operand << (self.ir_shift + 8)
        else:
            assert operand is None
        ir = sign_extend(ir & uword_max)
        if (
            (ir >> self.ir_shift) & 0xff == opcode and
            (operand is None or ir >> (self.ir_shift + 8) == operand)
        ):
            return ir
        return None

    def instruction(self, opcode, operand=None):
        '''
        Appends an instruction opcode.

         - opcode - an Instruction or an 8-bit integer.
         - operand - int - if `opcode` is in `TERMINAL_OPCODES`, the optional
           immediate operand, or `None`.
        '''
        # Compute the extended opcode.
        opcode = int(opcode)
        assert 0 <= opcode <= 0xff
        if operand is not None:
            assert opcode in TERMINAL_OPCODES

        # Store the extended opcode, starting a new word if necessary.
        self._fetch()
        ir = self.fit(opcode, operand)
        if ir is None: # Doesn't fit in the current word.
            self.label()
            self.word(0)
            ir = self.fit(opcode, operand)
            assert ir is not None
        self.state.M_word[self.ir_addr] = ir

        # Advance `self.ir_shift` past used bits.
        self.ir_shift += 8
        if opcode in TERMINAL_OPCODES:
            self.label()

    def jumprel(self, addr, opcode=I.JUMP):
        '''
        Assemble a relative `jump`, `jumpz` or `call` instruction to the given
        address. Selects the immediate form of the instruction if possible.
        '''
        assert opcode in (I.JUMP, I.JUMPZ, I.CALL)
        assert is_aligned(addr)
        # Compute value of `pc` that will be added to offset.
        self._fetch()
        word_offset = (addr - self.pc) // word_bytes
        if word_offset != 0 and self.fit(opcode, word_offset):
            # Use the immediate mode.
            self.instruction(opcode, word_offset)
        else:
            word_offset -= 1
            if sign_extend((word_offset << 8) & uword_max) >> 8 == word_offset:
                # Start a new word, and use the immediate mode.
                self.instruction(opcode, word_offset)
            else:
                # Don't start a new word. Use PUSHREL then the indirect mode.
                self.pushrel(addr)
                self.instruction(opcode)

    def push_long(self, value):
        '''
        Assemble a `push` instruction that pushes the specified `value`.
        '''
        self.instruction(I.PUSH)
        self.word(value)

    def push(self, value):
        '''
        Assemble a `push` instruction that pushes the specified `value`.
        Uses the shorter `pushi` if possible.
        '''
        value = int(value)
        if -32 <= value < 32:
            opcode = 0x3 if value >= 0 else 0x4
            self.instruction((opcode | (value << 3)) & 0xff)
        else:
            self.push_long(value)

    def pushrel_long(self, addr):
        '''
        Assemble a `pushrel` instruction that pushes the given address.
        See also `pushrel()`.
        '''
        assert is_aligned(addr)
        self.instruction(I.PUSHREL)
        offset = addr - self.pc
        word_offset = offset // word_bytes
        self.word(offset)

    def pushrel(self, addr):
        '''
        Assemble a `pushrel` instruction that pushes the given address.
        Uses the shorter `pushreli` if possible.
        '''
        assert is_aligned(addr)
        # Start a word if we need to.
        self._fetch()
        # If no room for an opcode in the current word, or offset is 64 (so
        # we can use PUSHRELI in the next word), move to next word.
        if not self.fit(I.PUSHRELI_0) or (addr - self.pc) // word_bytes == 64:
            self.label()
            self._fetch()
        offset = addr - self.pc
        word_offset = offset // word_bytes
        if -64 <= word_offset < 64:
            # 1-byte form.
            opcode = 0x1 if word_offset >= 0 else 0x2
            self.instruction((opcode | (word_offset << 2)) & 0xff)
        else:
            self.pushrel_long(addr)

    def extra(self, extra_code):
        '''
        Assemble the extra instruction given by `extra_code`.
        '''
        self.instruction(I.NEXT, extra_code)

    def trap(self, trap_code):
        '''
        Assemble the trap given by `trap_code`.
        '''
        self.instruction(I.NEXTFF, trap_code)
