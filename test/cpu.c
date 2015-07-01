
#include "common.h"

#include "test/test.h"

int main(int argc, char **argv)
{
    int ret;
    struct unit_test tests[] = {

    };

    ret = run_tests("Z80 CPU", tests, sizeof(tests) / sizeof(tests[0]), argc, argv);

    return ret;
}

