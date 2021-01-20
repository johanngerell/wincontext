#include <jg_string.h>
#include <jg_test.h>

jg::test_suite test_string_split()
{
    return
    {
        // positive
        [] {
            const auto tokens = jg::split<1>("", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "");
        },
        [] {
            const auto tokens = jg::split<1>("1", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "1");
        },
        [] {
            const auto tokens = jg::split<2>(",", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "");
            test_assert((*tokens)[1] == "");
        },
        [] {
            const auto tokens = jg::split<2>("1,", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "1");
            test_assert((*tokens)[1] == "");
        },
        [] {
            const auto tokens = jg::split<2>(",2", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "");
            test_assert((*tokens)[1] == "2");
        },
        [] {
            const auto tokens = jg::split<2>("1,2", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "1");
            test_assert((*tokens)[1] == "2");
        },
        [] {
            const auto tokens = jg::split<3>("1,2,", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "1");
            test_assert((*tokens)[1] == "2");
            test_assert((*tokens)[2] == "");
        },
        [] {
            const auto tokens = jg::split<3>(",2,3", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "");
            test_assert((*tokens)[1] == "2");
            test_assert((*tokens)[2] == "3");
        },
        [] {
            const auto tokens = jg::split<3>(",,", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "");
            test_assert((*tokens)[1] == "");
            test_assert((*tokens)[2] == "");
        },
        // negative
        [] {
            const auto tokens = jg::split<1>(",", ',');
            test_assert(!tokens.has_value());
        },
        [] {
            const auto tokens = jg::split<1>("1,", ',');
            test_assert(!tokens.has_value());
        },
        [] {
            const auto tokens = jg::split<1>("1,2", ',');
            test_assert(!tokens.has_value());
        },
        [] {
            const auto tokens = jg::split<2>(",,", ',');
            test_assert(!tokens.has_value());
        },
        [] {
            const auto tokens = jg::split<2>("1,2,", ',');
            test_assert(!tokens.has_value());
        },
        [] {
            const auto tokens = jg::split<2>("1,2,3", ',');
            test_assert(!tokens.has_value());
        }
    };
}
