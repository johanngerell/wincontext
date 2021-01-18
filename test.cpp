#include <cassert>
#include <iostream>
#include <functional>
#include <string>
#include "jg_string.h"

#ifdef NDEBUG
#error NDEBUG is defined and assert() is a no-op
#endif

using test_case = std::function<void()>;
using test_suite = std::vector<test_case>;
using test_suites = std::vector<test_suite>;

class ansi_color
{
public:
    const char* code() const { return m_code; }

protected:
    ansi_color(const char* code) : m_code{code} {}

private:
    const char* m_code{};
};

struct fg final : ansi_color
{
    using ansi_color::ansi_color;

    static fg normal()         { return "39"; }

    static fg black()          { return "30"; }
    static fg red()            { return "31"; }
    static fg green()          { return "32"; }
    static fg yellow()         { return "33"; }
    static fg blue()           { return "34"; }
    static fg magenta()        { return "35"; }
    static fg cyan()           { return "36"; }
    static fg white()          { return "37"; }

    static fg black_bright()   { return "90"; }
    static fg red_bright()     { return "91"; }
    static fg green_bright()   { return "92"; }
    static fg yellow_bright()  { return "93"; }
    static fg blue_bright()    { return "94"; }
    static fg magenta_bright() { return "95"; }
    static fg cyan_bright()    { return "96"; }
    static fg white_bright()   { return "97"; }
};

struct bg final : ansi_color
{
    using ansi_color::ansi_color;

    static bg normal()         { return "49"; }

    static bg black()          { return "40"; }
    static bg red()            { return "41"; }
    static bg green()          { return "42"; }
    static bg yellow()         { return "43"; }
    static bg blue()           { return "44"; }
    static bg magenta()        { return "45"; }
    static bg cyan()           { return "46"; }
    static bg white()          { return "47"; }

    static bg black_bright()   { return "100"; }
    static bg red_bright()     { return "101"; }
    static bg green_bright()   { return "102"; }
    static bg yellow_bright()  { return "103"; }
    static bg blue_bright()    { return "104"; }
    static bg magenta_bright() { return "105"; }
    static bg cyan_bright()    { return "106"; }
    static bg white_bright()   { return "107"; }
};

class ostream_color final
{
public:
    ostream_color(std::ostream& stream, fg f, bg b = bg::normal())
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
        ostream_color(std::cout, fg::green()) << "All tests succeeded\n";
        std::cout << "  " << test_count  << (test_count == 1 ? " test case" : " test cases") << "\n"
                  << "  " << suites.size() << (suites.size() == 1 ? " test suite" : " test suites") << "\n";
    }
    else
        ostream_color(std::cout, fg::yellow()) << "No test cases\n";
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
    test_run(
    {
        test_string_split()
    });
}