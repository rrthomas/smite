// Generate the specializer's predictor file.
//
// (c) Mit authors 2019
//
// The package is distributed under the MIT/X11 License.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "progname.h"

#include "mit/opcodes.h"

#include "warn.h"


#define HISTORY_BITS 20
#define NUM_HISTORIES (1 << HISTORY_BITS)
#define SPARSITY 3
#define NUM_OPCODES 32
#define COUNT_THRESHOLD 100



typedef uint32_t history_t;
typedef uint8_t opcode_t;

// Represents a function for updating the history.
struct step_function {
    history_t or_mask;
    history_t xor_mask;
};

static struct step_function step_functions[NUM_OPCODES];

// For `random_history()`.
static uint64_t seed;

// Internal to `init_step_functions()`.
// Generate a random HISTORY_BITS-bit number.
static history_t random_history(void) {
    // These odd 64-bit constants come from the digits of pi.
    seed ^= 0x98EC4E6C89452821ULL;
    seed *= 0x8A2E03707344A409ULL;
    return seed >> (64 - HISTORY_BITS);
}

static void init_step_functions(void) {
    for (opcode_t opcode = 0; opcode < NUM_OPCODES; opcode++) {
        seed = opcode;
        random_history();
        history_t or_mask = (1 << HISTORY_BITS) - 1;
        for (unsigned i = 0; i < SPARSITY; i++)
            or_mask &= random_history();
        step_functions[opcode].or_mask = or_mask;
        step_functions[opcode].xor_mask = random_history();
    }
}

static history_t step_function(history_t history, opcode_t opcode) {
    struct step_function sf = step_functions[opcode];
    return (history | sf.or_mask) ^ sf.xor_mask;
}

// How often each opcode has occurred after each history.
static uint64_t counts[NUM_HISTORIES][NUM_OPCODES];

static void init_counts(void) {
    for (history_t history = 0; history < NUM_HISTORIES; history++)
        for (opcode_t opcode = 0; opcode < NUM_OPCODES; opcode++)
            counts[history][opcode] = 0;
}

// Read a trace file and add its statistics to `counts`.
static void read_trace(FILE *trace) {
    history_t history = 0;
    while (1) {
        opcode_t opcode = fgetc(trace);
        if (feof(trace))
            break;
        assert(opcode < NUM_OPCODES);
        counts[history][opcode]++;
        history = step_function(history, opcode);
    }
}

// Maps common histories to their index in the output file.
// Uncommon histories are indicated by `-1`.
static int history_index[NUM_HISTORIES];

// Read `counts` and compute `history_index`.
// Returns the number of common histories.
static unsigned index_histories(void) {
    int num_common = 0;
    for (history_t history = 0; history < NUM_HISTORIES; history++) {
        uint64_t total = 0;
        for (opcode_t opcode = 0; opcode < NUM_OPCODES; opcode++)
            total += counts[history][opcode];
        history_index[history] = total >= COUNT_THRESHOLD ? num_common++ : -1;
    }
    return num_common;
}

// Write a predictor file containing just the histories that are common
// according to `history_index`.
static void write_predictor(FILE *fp) {
    fprintf(fp, "[");
    const char *list_sep = "";
    for (history_t history = 0; history < NUM_HISTORIES; history++) {
        int state = history_index[history];
        if (state != -1) {
            fprintf(fp, "%s\n    {", list_sep);
            list_sep = ", ";
            const char *dict_sep = "";
            for (opcode_t opcode = 0; opcode < NUM_OPCODES; opcode++) {
                uint64_t count = counts[history][opcode];
                history_t new_history = step_function(history, opcode);
                int new_state = history_index[new_history];
                if (new_state != -1) {
                    fprintf(
                        fp,
                        "%s\"%02x\": {\"new_state\": %d, \"count\": %"PRIu64"}",
                        dict_sep, opcode, new_state, count
                    );
                    dict_sep = ", ";
                }
            }
            fprintf(fp, "}");
        }
    }
    fprintf(fp, "\n]");
}

int main(int argc, char *argv[])
{
    set_program_name(argv[0]);
    if (argc < 3) {
        printf("Usage: %s TRACE-FILENAME PREDICTOR-FILENAME\n",
               program_name);
        exit(EXIT_SUCCESS);
    }
    const char *trace_filename = argv[1];
    const char *predictor_filename = argv[2];

    // Read input file.
    init_step_functions();
    init_counts();

    printf("Reading trace file '%s'.\n", trace_filename);
    FILE *trace = fopen(trace_filename, "rb");
    if (trace == NULL)
        die("%cannot not open file %s", trace_filename);
    read_trace(trace);
    fclose(trace);

    // Write output file.
    int num_common_histories = index_histories();
    printf("There are %d common history values.\n", num_common_histories);

    printf("Writing predictor file '%s'.\n", predictor_filename);
    FILE *fp = fopen(predictor_filename, "wb");
    if (trace == NULL)
        die("%cannot not open file %s", predictor_filename);
    write_predictor(fp);
    fclose(fp);
}