#include <chrono>
#include <string>
#include <vector>
#include <array>
#include <optional>
#include <algorithm>
#include <iostream>
#include <charconv>
#include "win32userdata.h"
#include "win32api.h"

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

template <typename Func>
std::chrono::nanoseconds benchmark(size_t sample_count, Func&& func)
{
    const auto t1 = std::chrono::high_resolution_clock::now();

    for (size_t sample = 0; sample < sample_count; ++sample)
        func();

    const auto t2 = std::chrono::high_resolution_clock::now();
    
    return std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1) / sample_count;
}

std::vector<HWND> g_labels;
std::vector<int> g_data;

std::string benchmark_userdata_access()
{
    constexpr size_t sample_count = 100;
    const std::vector<int> old_data = g_data;

    auto add_1 = [] (HWND label) { *static_cast<int*>(userdata_get(label)) += 1; };
    const auto sample_ns = benchmark(sample_count, [&] { for(HWND label : g_labels) add_1(label); });

    auto added_sample_count = [sample_count](int v1, int v2) { return v1 + sample_count == v2; };
    const bool mismatch = !std::equal(old_data.begin(), old_data.end(), g_data.begin(), g_data.end(), added_sample_count);
    
    if (mismatch)
        throw std::logic_error("Benchmark data mismatch");

    std::string text("average call time: ");
    text += std::to_string(sample_ns.count() / g_labels.size());
    text += " ns (";
    text += userdata_description();
    text += ")";

    return text;
}

LRESULT CALLBACK main_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_CHAR:
            switch (static_cast<char>(wp))
            {
                case 'g': SetWindowTextA(hwnd, benchmark_userdata_access().c_str()); break;
                case 'q': DestroyWindow(hwnd); break;
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, msg, wp, lp);
    }
}

HWND create_window(SIZE client_size)
{
    window_creation_info creation_info;
    creation_info.class_name = register_window_class(main_wndproc, "Main Window Class");
    creation_info.text       = "Press 'g' to measure userdata access";
    creation_info.style      = WS_POPUPWINDOW | WS_CAPTION;
    creation_info.size       = window_size_for_client(client_size, creation_info.style);
    creation_info.position   = {100, 100};

    return create_window(creation_info);
}

HWND create_label(HWND parent, POINT position, SIZE size)
{
    window_creation_info creation_info;
    creation_info.parent     = parent;
    creation_info.class_name = "STATIC";
    creation_info.style      = SS_BLACKFRAME;
    creation_info.size       = size;
    creation_info.position   = position;

    return create_window(creation_info);
}

std::vector<HWND> create_labels(HWND parent, const layout_info& layout, const grid_info& grid)
{
    std::vector<HWND> labels(grid.row_count * grid.column_count * grid.layer_count);

    std::generate(labels.begin(), labels.end(), [&, i = 0] () mutable
    {
        const auto cell = grid_cell_at(grid, i++);
        return create_label(parent, to_POINT(layout_cell_point(layout, cell)), to_SIZE(layout.cell_size));
    });

    return labels;
}

std::vector<int> create_labels_data()
{
    std::vector<int> data(g_labels.size());
    std::generate(data.begin(), data.end(), [] { return rand() % 4711; });

    return data;
}

void bind_userdata()
{
    for (size_t i = 0; i < g_labels.size(); ++i)
        userdata_set(g_labels[i], &g_data[i]);
}

constexpr bool starts_with(std::string_view string, std::string_view start)
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

struct args final
{
    int argc{};
    char** argv{};

    using iterator = char**;
    using const_iterator = const iterator;

    constexpr const_iterator begin() const { return &argv[0]; }
    constexpr const_iterator end() const { return begin() + argc; }
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

constexpr userdata_kind parse_userdata_kind(args a)
{
    if (const auto value = args_key_value(a, "option:"))
        if (const auto kind = from_chars<int, userdata_kind>(*value))
            return *kind;

    throw std::invalid_argument("Incorrect argument \"option:i\" where 'i' is in the interval [0, 6]");
}

constexpr grid_info parse_grid_info(args a)
{
    if (const auto value = args_key_value(a, "grid:"))
        if (const auto tokens = split<3>(*value, ','))
            if (const auto rows    = from_chars<size_t>((*tokens)[0]))
            if (const auto columns = from_chars<size_t>((*tokens)[1]))
            if (const auto layers  = from_chars<size_t>((*tokens)[2]))
                return { *rows, *columns, *layers };

    throw std::invalid_argument("Incorrect argument \"grid:i,j,k\" where 'i', 'j' and 'k' are rows, columns and layers");
}

constexpr layout_info parse_layout_info(args a)
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

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        app_options options{args{__argc, __argv}};
        userdata_init(options.kind);
        const HWND window = create_window(to_SIZE(layout_grid_size(options.layout, options.grid)));
        g_labels = create_labels(window, options.layout, options.grid);
        g_data = create_labels_data();
        bind_userdata();

        simple_message_loop();
    }
    catch(const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK);
    }

    return 0;
}