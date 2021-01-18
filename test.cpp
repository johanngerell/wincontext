#include <cassert>
#include <iostream>
#include "jg_string.h"

#ifdef NDEBUG
#error foobar
#endif

void test_string_split()
{
    // positive
    {
        const auto tokens = split<1>("", ',');
        assert(tokens.has_value());
        assert((*tokens)[0] == "");
    }
    {
        const auto tokens = split<1>("1", ',');
        assert(tokens.has_value());
        assert((*tokens)[0] == "1");
    }
    {
        const auto tokens = split<2>(",", ',');
        assert(tokens.has_value());
        assert((*tokens)[0] == "");
        assert((*tokens)[1] == "");
    }
    {
        const auto tokens = split<2>("1,", ',');
        assert(tokens.has_value());
        assert((*tokens)[0] == "1");
        assert((*tokens)[1] == "");
    }
    {
        const auto tokens = split<2>(",2", ',');
        assert(tokens.has_value());
        assert((*tokens)[0] == "");
        assert((*tokens)[1] == "2");
    }
    {
        const auto tokens = split<2>("1,2", ',');
        assert(tokens.has_value());
        assert((*tokens)[0] == "1");
        assert((*tokens)[1] == "2");
    }
    {
        const auto tokens = split<3>("1,2,", ',');
        assert(tokens.has_value());
        assert((*tokens)[0] == "1");
        assert((*tokens)[1] == "2");
        assert((*tokens)[2] == "");
    }
    {
        const auto tokens = split<3>(",2,3", ',');
        assert(tokens.has_value());
        assert((*tokens)[0] == "");
        assert((*tokens)[1] == "2");
        assert((*tokens)[2] == "3");
    }
    {
        const auto tokens = split<3>(",,", ',');
        assert(tokens.has_value());
        assert((*tokens)[0] == "");
        assert((*tokens)[1] == "");
        assert((*tokens)[2] == "");
    }

    // negative
    {
        const auto tokens = split<1>(",", ',');
        assert(!tokens.has_value());
    }
    {
        const auto tokens = split<1>("1,", ',');
        assert(!tokens.has_value());
    }
    {
        const auto tokens = split<1>("1,2", ',');
        assert(!tokens.has_value());
    }
    {
        const auto tokens = split<2>(",,", ',');
        assert(!tokens.has_value());
    }
    {
        const auto tokens = split<2>("1,2,", ',');
        assert(!tokens.has_value());
    }
    {
        const auto tokens = split<2>("1,2,3", ',');
        assert(!tokens.has_value());
    }

    std::cout << "All string split tests OK!\n";
}

int main()
{
    test_string_split();

    std::cout << "All tests OK!\n";
}