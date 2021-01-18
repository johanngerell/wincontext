#include <cassert>
#include <iostream>
#include <functional>
#include <string>
#include "jg_string.h"

#ifdef NDEBUG
#error NDEBUG is defined ans assert() is a nop
#endif

using test_case = std::function<void()>;
using test_suite = std::vector<test_case>;
using test_suites = std::vector<test_suite>;

struct color_green final
{
    static const char* fg() { return "32"; }
};

struct color_yellow final
{
    static const char* fg() { return "33"; }
};

struct color_default final
{
    static const char* bg() { return "49"; }
};

template <typename FG, typename BG = color_default>
class ansi_color final
{
    std::ostream& m_stream;

public:
    ansi_color(std::ostream& stream)
        : m_stream{stream}
    {
        m_stream << "\033[" << FG::fg() << ";" << BG::bg() << "m";
    }

    template <typename T>
    friend std::ostream& operator<<(const ansi_color<typename FG, typename BG>& self, const T& t)
    {
        self.m_stream << t;
        return self.m_stream;
    }

    ~ansi_color()
    {
        m_stream << "\033[0m";
    }
};

void test_run(const test_suites& suites)
{
    size_t test_count = 0;

    for (auto& suite : suites)
        for (auto& test : suite)
        {
            ++test_count;
            test();
        }

    if (test_count > 0)
    {
        const char* test_case_string = test_count == 1 ? " test case" : " test cases";
        const char* suite_string = suites.size() == 1 ? " test suite" : " test suites";
        ansi_color<color_green>(std::cout) << "All tests succeeded\n";
        std::cout << "  " << test_count  << test_case_string << "\n"
                  << "  " << suites.size() << suite_string     << "\n";
    }
    else
        ansi_color<color_yellow>(std::cout) << "No test cases\n";
}

test_suite test_string_split()
{
    return
    {
        // positive
        [] {
            const auto tokens = split<1>("", ',');
            assert(tokens.has_value());
            assert((*tokens)[0] == "");
        },
        [] {
            const auto tokens = split<1>("1", ',');
            assert(tokens.has_value());
            assert((*tokens)[0] == "1");
        },
        [] {
            const auto tokens = split<2>(",", ',');
            assert(tokens.has_value());
            assert((*tokens)[0] == "");
            assert((*tokens)[1] == "");
        },
        [] {
            const auto tokens = split<2>("1,", ',');
            assert(tokens.has_value());
            assert((*tokens)[0] == "1");
            assert((*tokens)[1] == "");
        },
        [] {
            const auto tokens = split<2>(",2", ',');
            assert(tokens.has_value());
            assert((*tokens)[0] == "");
            assert((*tokens)[1] == "2");
        },
        [] {
            const auto tokens = split<2>("1,2", ',');
            assert(tokens.has_value());
            assert((*tokens)[0] == "1");
            assert((*tokens)[1] == "2");
        },
        [] {
            const auto tokens = split<3>("1,2,", ',');
            assert(tokens.has_value());
            assert((*tokens)[0] == "1");
            assert((*tokens)[1] == "2");
            assert((*tokens)[2] == "");
        },
        [] {
            const auto tokens = split<3>(",2,3", ',');
            assert(tokens.has_value());
            assert((*tokens)[0] == "");
            assert((*tokens)[1] == "2");
            assert((*tokens)[2] == "3");
        },
        [] {
            const auto tokens = split<3>(",,", ',');
            assert(tokens.has_value());
            assert((*tokens)[0] == "");
            assert((*tokens)[1] == "");
            assert((*tokens)[2] == "");
        },
        // negative
        [] {
            const auto tokens = split<1>(",", ',');
            assert(!tokens.has_value());
        },
        [] {
            const auto tokens = split<1>("1,", ',');
            assert(!tokens.has_value());
        },
        [] {
            const auto tokens = split<1>("1,2", ',');
            assert(!tokens.has_value());
        },
        [] {
            const auto tokens = split<2>(",,", ',');
            assert(!tokens.has_value());
        },
        [] {
            const auto tokens = split<2>("1,2,", ',');
            assert(!tokens.has_value());
        },
        [] {
            const auto tokens = split<2>("1,2,3", ',');
            assert(!tokens.has_value());
        }
    };
}

int main()
{
    test_run({
        test_string_split()
    });

    /*
    printf("\n");

    printf("\033[30mBlack FG\033[0m           "); 
    printf("\033[31mRed FG\033[0m             "); 
    printf("\033[32mGreen FG\033[0m           ");
    printf("\033[33mYellow FG\033[0m          ");
    printf("\033[34mBlue FG\033[0m            ");
    printf("\033[35mMagenta FG\033[0m         ");
    printf("\033[36mCyan FG\033[0m            ");
    printf("\033[37mWhite FG\033[0m\n");

    printf("\033[90mBright Black FG\033[0m    "); 
    printf("\033[91mBright Red FG\033[0m      "); 
    printf("\033[92mBright Green FG\033[0m    ");
    printf("\033[93mBright Yellow FG\033[0m   ");
    printf("\033[94mBright Blue FG\033[0m     ");
    printf("\033[95mBright Magenta FG\033[0m  ");
    printf("\033[96mBright Cyan FG\033[0m     ");
    printf("\033[97mBright White FG\033[0m\n");

    printf("\033[30;47mBlack FG\033[0m           "); 
    printf("\033[31;47mRed FG\033[0m             "); 
    printf("\033[32;47mGreen FG\033[0m           ");
    printf("\033[33;47mYellow FG\033[0m          ");
    printf("\033[34;47mBlue FG\033[0m            ");
    printf("\033[35;47mMagenta FG\033[0m         ");
    printf("\033[36;47mCyan FG\033[0m            ");
    printf("\033[37;47mWhite FG\033[0m\n");

    printf("\n");
    */
}