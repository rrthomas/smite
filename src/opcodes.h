// enum type for the opcodes to make the interpreter more readable. Opcode
// names which are not valid C identifiers have been altered.
//
// (c) Reuben Thomas 1994-2018
//
// The package is distributed under the GNU Public License version 3, or,
// at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER‘S
// RISK.

#ifndef PACKAGE_UPPER_OPCODES
#define PACKAGE_UPPER_OPCODES


enum {
    O_NEXT00,
    O_DUP,
    O_DROP,
    O_SWAP,
    O_OVER,
    O_ROT,
    O_UNDEF40,
    O_UNDEF41,
    O_UNDEF42,
    O_PICK,
    O_ROLL,
    O_UNDEF0,
    O_TOR,
    O_RFROM,
    O_RFETCH,
    O_LESS,
    O_UNDEF1,
    O_EQUAL,
    O_UNDEF2,
    O_UNDEF3,
    O_UNDEF4,
    O_UNDEF5,
    O_UNDEF6,
    O_ULESS,
    O_UNDEF7,
    O_UNDEF8,
    O_UNDEF9,
    O_UNDEF10,
    O_UNDEF11,
    O_UNDEF12,
    O_PLUS,
    O_UNDEF13,
    O_UNDEF14,
    O_UNDEF15,
    O_UNDEF16,
    O_UNDEF17,
    O_UNDEF18,
    O_STAR,
    O_SLASH,
    O_MOD,
    O_SLASHMOD,
    O_USLASHMOD,
    O_SSLASHREM,
    O_UNDEF19,
    O_UNDEF20,
    O_UNDEF21,
    O_NEGATE,
    O_UNDEF22,
    O_UNDEF23,
    O_INVERT,
    O_AND,
    O_OR,
    O_XOR,
    O_LSHIFT,
    O_RSHIFT,
    O_UNDEF24,
    O_UNDEF25,
    O_FETCH,
    O_STORE,
    O_CFETCH,
    O_CSTORE,
    O_UNDEF26,
    O_SPFETCH,
    O_SPSTORE,
    O_RPFETCH,
    O_RPSTORE,
    O_EPFETCH,
    O_S0FETCH,
    O_HASHS,
    O_R0FETCH,
    O_HASHR,
    O_THROWFETCH,
    O_THROWSTORE,
    O_MEMORYFETCH,
    O_BADFETCH,
    O_NOT_ADDRESSFETCH,
    O_BRANCH,
    O_UNDEF27,
    O_QBRANCH,
    O_UNDEF28,
    O_EXECUTE,
    O_UNDEF29,
    O_CALL,
    O_UNDEF,
    O_EXIT,
    O_UNDEF30,
    O_UNDEF31,
    O_UNDEF32,
    O_UNDEF33,
    O_UNDEF34,
    O_UNDEF35,
    O_J,
    O_LITERAL,
    O_UNDEF36,
    O_THROW,
    O_HALT,
    O_LINK,
    O_UNDEFINED = 0x7f,
    OX_ARGC = 0x80,
    OX_ARG,
    OX_STDIN,
    OX_STDOUT,
    OX_STDERR,
    OX_OPEN_FILE,
    OX_CLOSE_FILE,
    OX_READ_FILE,
    OX_WRITE_FILE,
    OX_FILE_POSITION,
    OX_REPOSITION_FILE,
    OX_FLUSH_FILE,
    OX_RENAME_FILE,
    OX_DELETE_FILE,
    OX_FILE_SIZE,
    OX_RESIZE_FILE,
    OX_FILE_STATUS,
    O_NEXTFF = 0xff
};


#endif
