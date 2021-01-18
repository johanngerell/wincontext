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
    static_assert(NumTokens > 0);

    std::array<std::string_view, NumTokens> tokens;

    for (size_t i = 0; i < NumTokens; ++i)
    {
        const size_t offset = string.find(delimiter);

        if (offset != std::string_view::npos && i == NumTokens - 1)
            return std::nullopt;

        if (offset == std::string_view::npos && i != NumTokens - 1)
            return std::nullopt;

        tokens[i] = string.substr(0, offset);

        if (offset != std::string_view::npos)
            string.remove_prefix(offset + 1);
    }

    return tokens;
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
