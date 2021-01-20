#pragma once

#include <stdexcept>
#include "win32_userdata.h"
#include "jg_args.h"
#include "app_types.h"

constexpr userdata_kind parse_userdata_kind(jg::args args)
{
    if (const auto value = jg::args_key_value(args, "option:"))
        if (const auto kind = jg::from_chars<int, userdata_kind>(*value))
            return *kind;

    throw std::invalid_argument("Incorrect argument \"option:i\" where 'i' is in the interval [0, 7]");
}

constexpr grid_info parse_grid_info(jg::args args)
{
    if (const auto value = jg::args_key_value(args, "grid:"))
        if (const auto tokens = jg::split<3>(*value, ','))
            if (const auto rows    = jg::from_chars<size_t>((*tokens)[0]))
            if (const auto columns = jg::from_chars<size_t>((*tokens)[1]))
            if (const auto layers  = jg::from_chars<size_t>((*tokens)[2]))
                return { *rows, *columns, *layers };

    throw std::invalid_argument("Incorrect argument \"grid:i,j,k\" where 'i', 'j' and 'k' are rows, columns and layers");
}

constexpr layout_info parse_layout_info(jg::args args)
{
    if (const auto value = jg::args_key_value(args, "layout:"))
        if (const auto tokens = jg::split<3>(*value, ','))
            if (const auto cell_spacing = jg::from_chars<size_t>((*tokens)[0]))
            if (const auto cell_width   = jg::from_chars<size_t>((*tokens)[1]))
            if (const auto cell_height  = jg::from_chars<size_t>((*tokens)[2]))
                return { *cell_spacing, *cell_width, *cell_height };

    throw std::invalid_argument("Incorrect argument \"layout:i,j,k\" where 'i', 'j' and 'k' are cell spacing, cell width and cell height");
}

struct app_options final
{   
    userdata_kind kind;
    grid_info     grid;
    layout_info   layout;

    app_options(jg::args args)
        : kind  {parse_userdata_kind(args)}
        , grid  {parse_grid_info(args)}
        , layout{parse_layout_info(args)}
    {}
};
