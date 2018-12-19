// Encode and decode instructions.
//
// (c) Reuben Thomas 2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#include "config.h"

#include "minmax.h"

#include "external_syms.h"

#include "public.h"
#include "aux.h"
#include "opcodes.h"


#define _ENCODE_INSTRUCTION(NAME, TYPE, HANDLE, STORE)                  \
    {                                                                   \
        size_t len = 0;                                                 \
        int exception;                                                  \
                                                                        \
        /* Continuation bytes */                                        \
        for (int bits = find_msbit(v) + 1; bits > INSTRUCTION_CHUNK_BIT; bits -= INSTRUCTION_CHUNK_BIT) { \
            if ((exception = STORE((BYTE)(v & INSTRUCTION_CHUNK_MASK) | 0x40)) != 0) \
                return (ptrdiff_t)exception;                            \
            len++;                                                      \
            v = ARSHIFT(v, INSTRUCTION_CHUNK_BIT);                      \
        }                                                               \
                                                                        \
        /* Set action bit if needed */                                  \
        if (type == INSTRUCTION_ACTION)                                 \
            v |= 0x80;                                                  \
                                                                        \
        /* Last (or only) byte */                                       \
        if ((exception = STORE((BYTE)v)))                               \
            return (ptrdiff_t)exception;                                \
        len++;                                                          \
                                                                        \
        return len;                                                     \
    }

#define ENCODE_INSTRUCTION(NAME, TYPE, HANDLE, STORE)                   \
    ptrdiff_t NAME(TYPE HANDLE, enum instruction_type type, WORD v)     \
    _ENCODE_INSTRUCTION(NAME, TYPE, HANDLE, STORE)

#define STATEFUL_ENCODE_INSTRUCTION(NAME, TYPE, HANDLE, STORE)          \
    ptrdiff_t NAME(state *S, TYPE HANDLE, enum instruction_type type, WORD v) \
    _ENCODE_INSTRUCTION(NAME, TYPE, HANDLE, STORE)

#define STORE_NATIVE(b) (*addr++ = (b), 0)
ENCODE_INSTRUCTION(encode_instruction_native, BYTE *, addr, STORE_NATIVE)

#define STORE_VIRTUAL(b) (store_byte(S, (*addr)++, (b)))
STATEFUL_ENCODE_INSTRUCTION(encode_instruction, UWORD *, addr, STORE_VIRTUAL)

#define _DECODE_INSTRUCTION(NAME, TYPE, HANDLE, LOAD)                   \
    {                                                                   \
        size_t len = 0;                                                 \
        unsigned bits = 0;                                              \
        WORD n = 0;                                                     \
        BYTE b;                                                         \
        int exception;                                                  \
        int type = INSTRUCTION_NUMBER;                                  \
                                                                        \
        /* Continuation bytes */                                        \
        for (exception = LOAD(b); exception == 0 && (b & ~INSTRUCTION_CHUNK_MASK) == 0x40; exception = LOAD(b)) { \
            len++;                                                      \
            n |= (WORD)(b & INSTRUCTION_CHUNK_MASK) << bits;            \
            bits += INSTRUCTION_CHUNK_BIT;                              \
        }                                                               \
        if (exception != 0)                                             \
            return exception;                                           \
                                                                        \
        /* Check for action opcode */                                   \
        if ((b & ~INSTRUCTION_CHUNK_MASK) == 0x80) {                    \
            type = INSTRUCTION_ACTION;                                  \
            b &= INSTRUCTION_CHUNK_MASK;                                \
        }                                                               \
                                                                        \
        /* Final (or only) byte */                                      \
        len++;                                                          \
        n |= (WORD)b << bits;                                           \
        bits = MIN(bits + BYTE_BIT, WORD_BIT);                          \
        if (type == INSTRUCTION_NUMBER && bits < WORD_BIT)              \
            n = ARSHIFT(n << (WORD_BIT - bits), WORD_BIT - bits);       \
        *val = n;                                                       \
        return type;                                                    \
    }

#define DECODE_INSTRUCTION(NAME, TYPE, HANDLE, LOAD)                    \
    ptrdiff_t NAME(TYPE HANDLE, WORD *val)                              \
    _DECODE_INSTRUCTION(NAME, TYPE, HANDLE, LOAD)

#define STATEFUL_DECODE_INSTRUCTION(NAME, TYPE, HANDLE, LOAD)           \
    ptrdiff_t NAME(state *S, TYPE HANDLE, WORD *val)                    \
    _DECODE_INSTRUCTION(NAME, TYPE, HANDLE, LOAD)

#define LOAD_FILE(b) (-((b = getc(file)) == EOF))
DECODE_INSTRUCTION(decode_instruction_file, FILE *, file, LOAD_FILE)

#define LOAD_VIRTUAL(b) (load_byte(S, (*addr)++, &(b)))
STATEFUL_DECODE_INSTRUCTION(decode_instruction, UWORD *, addr, LOAD_VIRTUAL)