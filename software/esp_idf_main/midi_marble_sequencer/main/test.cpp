#include "test.h"

#include <stdio.h>

Test::Test()
{
    _x = 14;
}

void Test::print_x()
{
    printf("XXX: %d\n", _x);
    fflush(stdout);
}
