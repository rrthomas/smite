/*
    Test the branch instructions. Also uses other instructions with lower
    opcodes than the instructions tested (i.e. those already tested).
    See exceptions.c for address exception handling tests.
    The test program contains an infinite loop, but this is only executed
    once.

    (c) Reuben Thomas 1994-2018
*/


#include "btests.h"


unsigned correct[] = { 4, 100, 52, 10004, 10004, 10008, 10008, 10012, 10012, 11004,
                       11004, 11020, 11024, 68, 204, 304, 212, 72, 76, 80, 80, 80, 68 };


int main(void)
{
    int exception = 0;

    size_t size = 4096;
    init_beetle((CELL *)calloc(size, CELL_W), size);

    here = EP;	/* start assembling at 0 */
    start_ass();
    ass(O_BRANCHI); ilit(23);
    end_ass();
    instrs++;	/* correct instrs after final immediate literal */

    here = 96;	/* start assembling at 96 */
    start_ass();
    ass(O_BRANCHI); ilit(-13);
    end_ass();
    instrs++;	/* correct instrs after final immediate literal */

    here = 48;	/* start assembling at 48 */
    start_ass();
    ass(O_BRANCH); lit(10000);
    end_ass();

    here = 10000;    /* start assembling at 10000 */
    start_ass();
    ass(O_ONE); ass(O_QBRANCHI); ilit(0);
    ass(O_ONE); ass(O_QBRANCH); lit(0); ass(O_ZERO); ass(O_QBRANCH); lit(11000);
    end_ass();

    here = 11000;    /* start assembling at 11000 */
    start_ass();
    ass(O_ZERO); ass(O_QBRANCHI); ilit(3);
    end_ass();
    instrs++;	/* correct instrs after final immediate literal */

    here = 11016;    /* start assembling at 11016 */
    start_ass();
    ass(O_LITERALI); ilit(64);
    ass(O_EXECUTE);
    end_ass();

    here = 64;	/* start assembling at 64 */
    start_ass();
    ass(O_CALLI); ilit(33);
    ass(O_LITERALI); ilit(64);
    ass(O_LITERALI); ilit(20);
    ass(O_TUCK); ass(O_STORE); ass(O_FEXECUTE);
    end_ass();

    here = 200;	/* start assembling at 200 */
    start_ass();
    ass(O_CALL); lit(300); ass(O_NEXT00); ass(O_NEXT00); ass(O_NEXT00);
    ass(O_EXIT);
    end_ass();
    instrs -= 3; /* correct instrs for NEXT00s */

    here = 300;	/* start assembling at 300 */
    start_ass();
    ass(O_EXIT);
    end_ass();

    NEXT;   /* load first instruction word */

    assert(instrs == sizeof(correct) / sizeof(correct[0]));
    for (int i = 0; i < instrs; i++) {
        printf("Instruction %d: EP = %u; should be %u\n\n", i, EP, correct[i]);
        if (correct[i] != EP) {
            printf("Error in branch tests: EP = %"PRIu32"\n", EP);
            exit(1);
        }
        single_step();
        printf("I = %s\n", disass(I));
    }

    assert(exception == 0);
    printf("Branch tests ran OK\n");
    return 0;
}
