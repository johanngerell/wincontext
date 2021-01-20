#include <jg_string.h>
#include <jg_test.h>
#include "test_string_split.h"

int main()
{
    return jg::test_run(
    {
        test_string_split()
    });
}