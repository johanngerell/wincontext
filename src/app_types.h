#pragma once

#include "win32_api.h"

struct grid_info final
{
    size_t row_count{};
    size_t column_count{};
    size_t layer_count{};
};

struct cell_info final
{
    size_t row_index{};
    size_t column_index{};
    size_t layer_index{};
};

struct layout_point final
{
    size_t x{};
    size_t y{};
};

struct layout_size final
{
    size_t width{};
    size_t height{};
};

struct layout_info final
{
    size_t cell_spacing{};
    layout_size cell_size{};
};

constexpr cell_info grid_cell_at(const grid_info& grid, size_t index)
{
    return
    {
        (index % (grid.row_count * grid.column_count)) / grid.column_count,
        (index % (grid.row_count * grid.column_count)) % grid.column_count,
         index / (grid.row_count * grid.column_count)
    };
}

constexpr POINT to_POINT(const layout_point& point)
{
    return {static_cast<LONG>(point.x), static_cast<LONG>(point.y)};
};

constexpr SIZE to_SIZE(const layout_size& size)
{
    return {static_cast<LONG>(size.width), static_cast<LONG>(size.height)};
};

constexpr layout_point layout_cell_point(const layout_info& layout, const cell_info& cell)
{
    return
    {
        cell.row_index    * (layout.cell_size.height + layout.cell_spacing) + layout.cell_spacing + cell.layer_index * 2,
        cell.column_index * (layout.cell_size.width + layout.cell_spacing)  + layout.cell_spacing + cell.layer_index * 2
    };
}

constexpr layout_size layout_grid_size(const layout_info& layout, const grid_info& grid)
{
    return
    {
        grid.column_count * (layout.cell_size.width + layout.cell_spacing)  + layout.cell_spacing + grid.layer_count * 2,
        grid.row_count    * (layout.cell_size.height + layout.cell_spacing) + layout.cell_spacing + grid.layer_count * 2
    };
};
