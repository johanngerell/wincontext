#pragma once

#include "jg_string.h"

namespace jg
{

class args final
{
public:
    args() = default;
    args(int argc, char** argv) : argc{argc}, argv{argv} {}

    using iterator = char**;
    using const_iterator = const iterator;

    constexpr const_iterator begin() const { return &argv[0]; }
    constexpr const_iterator end() const { return begin() + argc; }

private:
    int argc{};
    char** argv{};
};

constexpr std::optional<std::string_view> arg_key_value(std::string_view arg, std::string_view key)
{
    if (starts_with(arg, key))
        return arg.substr(key.length());
    
    return std::nullopt;
}

constexpr std::optional<std::string_view> args_key_value(args a, std::string_view key)
{
    for (auto arg : a)
        if (auto value = arg_key_value(arg, key))
            return value;

    return std::nullopt;
}

} // namespace jg