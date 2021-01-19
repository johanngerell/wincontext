#include <iostream>
#include <functional>
#include <string>
#include "jg_string.h"

template <typename Tag>
class ansi_color
{
public:
    ansi_color(const char* code) : m_code{code} {}
    const char* code() const { return m_code; }

private:
    const char* m_code{};
};

struct fg_tag{};
using fg = ansi_color<fg_tag>;

struct bg_tag{};
using bg = ansi_color<bg_tag>;

fg fg_normal()         { return "39"; }
fg fg_black()          { return "30"; }
fg fg_red()            { return "31"; }
fg fg_green()          { return "32"; }
fg fg_yellow()         { return "33"; }
fg fg_blue()           { return "34"; }
fg fg_magenta()        { return "35"; }
fg fg_cyan()           { return "36"; }
fg fg_white()          { return "37"; }
fg fg_black_bright()   { return "90"; }
fg fg_red_bright()     { return "91"; }
fg fg_green_bright()   { return "92"; }
fg fg_yellow_bright()  { return "93"; }
fg fg_blue_bright()    { return "94"; }
fg fg_magenta_bright() { return "95"; }
fg fg_cyan_bright()    { return "96"; }
fg fg_white_bright()   { return "97"; }

bg bg_normal()         { return "49"; }
bg bg_black()          { return "40"; }
bg bg_red()            { return "41"; }
bg bg_green()          { return "42"; }
bg bg_yellow()         { return "43"; }
bg bg_blue()           { return "44"; }
bg bg_magenta()        { return "45"; }
bg bg_cyan()           { return "46"; }
bg bg_white()          { return "47"; }
bg bg_black_bright()   { return "100"; }
bg bg_red_bright()     { return "101"; }
bg bg_green_bright()   { return "102"; }
bg bg_yellow_bright()  { return "103"; }
bg bg_blue_bright()    { return "104"; }
bg bg_magenta_bright() { return "105"; }
bg bg_cyan_bright()    { return "106"; }
bg bg_white_bright()   { return "107"; }

class ostream_color final
{
public:
    ostream_color(std::ostream& stream, fg f, bg b = bg_normal())
        : m_stream{stream}
    {
        m_stream << "\033[" << f.code() << ";" << b.code() << "m";
    }

    template <typename T>
    friend std::ostream& operator<<(const ostream_color& self, const T& t)
    {
        self.m_stream << t;
        return self.m_stream;
    }

    ~ostream_color()
    {
        m_stream << "\033[0m";
    }

private:
    std::ostream& m_stream;
};

struct test_statistics final
{
    size_t suite_count{};
    size_t case_count{};
    size_t assertion_count{};
    size_t case_fail_count{};
    size_t assertion_fail_count{};
};

test_statistics* g_statistics{};

using test_case = std::function<void()>;
using test_suite = std::vector<test_case>;
using test_suites = std::vector<test_suite>;

int test_run(const test_suites& suites)
{
    test_statistics statistics{};
    g_statistics = &statistics;

    statistics.suite_count = suites.size();

    for (auto& suite : suites)
    {
        statistics.case_count += suite.size();

        for (auto& test : suite)
        {
            const size_t assertion_fail_count_before = statistics.assertion_fail_count;
            test();

            if (statistics.assertion_fail_count > assertion_fail_count_before)
                statistics.case_fail_count++;
        }
    }

    g_statistics = nullptr;

    if (statistics.case_count == 0)
        ostream_color(std::cout, fg_yellow()) << "No test cases\n";
    else
    {
        if (statistics.assertion_fail_count == 0)
            ostream_color(std::cout, fg_green()) << "All tests succeeded\n";

        std::cout << "  " << statistics.assertion_count  << (statistics.assertion_count == 1 ? " test assertion" : " test assertions") << "\n"
                  << "  " << statistics.case_count       << (statistics.case_count == 1 ? " test case" : " test cases") << "\n"
                  << "  " << statistics.suite_count      << (statistics.suite_count == 1 ? " test suite" : " test suites") << "\n";
    }

    return static_cast<int>(statistics.assertion_fail_count);
}

inline void test_assert_impl(bool expr_value, const char* expr_string, const char* file, int line)
{
    // Allowing g_statistics to be unset makes it possible to use the test_assert
    // macro outside of test suites, which is useful for quick main()-only tests
    // that often grow to more complete suites.

    if (g_statistics)
        g_statistics->assertion_count++;
    
    if (expr_value)
        return;

    if (g_statistics)
        g_statistics->assertion_fail_count++;
    
    ostream_color(std::cout, fg_red()) << "Test assertion '" << expr_string << "' failed at "
                                       << file << ":" << line << "\n";
}

#define test_assert(expr) test_assert_impl((expr), #expr, __FILE__,  __LINE__) 

test_suite test_string_split()
{
    return
    {
        // positive
        [] {
            const auto tokens = split<1>("", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "");
        },
        [] {
            const auto tokens = split<1>("1", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "1");
        },
        [] {
            const auto tokens = split<2>(",", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "");
            test_assert((*tokens)[1] == "");
        },
        [] {
            const auto tokens = split<2>("1,", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "1");
            test_assert((*tokens)[1] == "");
        },
        [] {
            const auto tokens = split<2>(",2", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "");
            test_assert((*tokens)[1] == "2");
        },
        [] {
            const auto tokens = split<2>("1,2", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "1");
            test_assert((*tokens)[1] == "2");
        },
        [] {
            const auto tokens = split<3>("1,2,", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "1");
            test_assert((*tokens)[1] == "2");
            test_assert((*tokens)[2] == "");
        },
        [] {
            const auto tokens = split<3>(",2,3", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "");
            test_assert((*tokens)[1] == "2");
            test_assert((*tokens)[2] == "3");
        },
        [] {
            const auto tokens = split<3>(",,", ',');
            test_assert(tokens.has_value());
            test_assert((*tokens)[0] == "");
            test_assert((*tokens)[1] == "");
            test_assert((*tokens)[2] == "");
        },
        // negative
        [] {
            const auto tokens = split<1>(",", ',');
            test_assert(!tokens.has_value());
        },
        [] {
            const auto tokens = split<1>("1,", ',');
            test_assert(!tokens.has_value());
        },
        [] {
            const auto tokens = split<1>("1,2", ',');
            test_assert(!tokens.has_value());
        },
        [] {
            const auto tokens = split<2>(",,", ',');
            test_assert(!tokens.has_value());
        },
        [] {
            const auto tokens = split<2>("1,2,", ',');
            test_assert(!tokens.has_value());
        },
        [] {
            const auto tokens = split<2>("1,2,3", ',');
            test_assert(!tokens.has_value());
        }
    };
}

int main()
{
    return test_run(
    {
        test_string_split()
    });
}