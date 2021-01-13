#pragma once

#include <array>
#include <charconv>
#include <optional>
#include <string_view>

inline constexpr bool starts_with(std::string_view string, std::string_view start)
{
    return string.rfind(start, 0) == 0;
}

template <size_t NumTokens>
constexpr std::optional<std::array<std::string_view, NumTokens>> split(std::string_view string, char delimiter)
{
    std::array<std::string_view, NumTokens> tokens;
    size_t i = 0;

    while (true)
    {
        if (const size_t offset = string.find(delimiter); offset != std::string_view::npos)
        {
            if (i >= NumTokens)
                return std::nullopt;

            tokens[i++] = string.substr(0, offset);
            string.remove_prefix(offset + 1);
        }
        else
        {
            tokens[i++] = std::move(string);
            break;
        }
    }

    if (i == NumTokens)
        return tokens;

    return std::nullopt;
}

template <typename T, typename U = T>
constexpr std::optional<U> from_chars(std::string_view string)
{
    T value{};
    auto [_, result] = std::from_chars(string.data(), string.data() + string.size(), value);

    if (result == std::errc())
        return static_cast<U>(value);

    return std::nullopt;
}
