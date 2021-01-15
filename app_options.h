#pragma once

#include <stdexcept>
#include "win32_userdata.h"
#include "jg_args.h"
#include "app_types.h"

inline constexpr userdata_kind parse_userdata_kind(args a)
{
    if (const auto value = args_key_value(a, "option:"))
        if (const auto kind = from_chars<int, userdata_kind>(*value))
            return *kind;

    throw std::invalid_argument("Incorrect argument \"option:i\" where 'i' is in the interval [0, 7]");
}

inline constexpr grid_info parse_grid_info(args a)
{
    if (const auto value = args_key_value(a, "grid:"))
        if (const auto tokens = split<3>(*value, ','))
            if (const auto rows    = from_chars<size_t>((*tokens)[0]))
            if (const auto columns = from_chars<size_t>((*tokens)[1]))
            if (const auto layers  = from_chars<size_t>((*tokens)[2]))
                return { *rows, *columns, *layers };

    throw std::invalid_argument("Incorrect argument \"grid:i,j,k\" where 'i', 'j' and 'k' are rows, columns and layers");
}

inline constexpr layout_info parse_layout_info(args a)
{
    if (const auto value = args_key_value(a, "layout:"))
        if (const auto tokens = split<3>(*value, ','))
            if (const auto cell_spacing = from_chars<size_t>((*tokens)[0]))
            if (const auto cell_width   = from_chars<size_t>((*tokens)[1]))
            if (const auto cell_height  = from_chars<size_t>((*tokens)[2]))
                return { *cell_spacing, *cell_width, *cell_height };

    throw std::invalid_argument("Incorrect argument \"layout:i,j,k\" where 'i', 'j' and 'k' are cell spacing, cell width and cell height");
}

class app_options final
{
    args a;

public:
    app_options(args a) : a{a} {}
    userdata_kind kind  {parse_userdata_kind(a)};
    grid_info     grid  {parse_grid_info(a)};
    layout_info   layout{parse_layout_info(a)};
};
