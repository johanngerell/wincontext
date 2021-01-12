#include <chrono>
#include <string>
#include <vector>
#include <algorithm>
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

cell_info grid_cell_at(const grid_info& grid, size_t index)
{
    return
    {
        (index % (grid.row_count * grid.column_count)) / grid.column_count,
        (index % (grid.row_count * grid.column_count)) % grid.column_count,
         index / (grid.row_count * grid.column_count)
    };
}

POINT to_POINT(const layout_point& point)
{
    return {static_cast<LONG>(point.x), static_cast<LONG>(point.y)};
};

SIZE to_SIZE(const layout_size& size)
{
    return {static_cast<LONG>(size.width), static_cast<LONG>(size.height)};
};

layout_point layout_cell_point(const layout_info& layout, const cell_info& cell)
{
    return
    {
        cell.row_index    * (layout.cell_size.height + layout.cell_spacing) + layout.cell_spacing + cell.layer_index * 2,
        cell.column_index * (layout.cell_size.width + layout.cell_spacing)  + layout.cell_spacing + cell.layer_index * 2
    };
}

layout_size layout_grid_size(const layout_info& layout, const grid_info& grid)
{
    return
    {
        grid.column_count * (layout.cell_size.width + layout.cell_spacing)  + layout.cell_spacing + grid.layer_count * 2,
        grid.row_count    * (layout.cell_size.height + layout.cell_spacing) + layout.cell_spacing + grid.layer_count * 2
    };
};

template <typename Func>
std::chrono::nanoseconds::rep benchmark(size_t sample_count, Func&& func)
{
    const auto t1 = std::chrono::high_resolution_clock::now();

    for (size_t sample = 0; sample < sample_count; ++sample)
        func();

    const auto t2 = std::chrono::high_resolution_clock::now();
    
    return std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / sample_count;
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
        throw std::logic_error("Data mismatch");

    std::string text("average call time: ");
    text += std::to_string(sample_ns / g_labels.size());
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

userdata_kind parse_command_line()
{
    if (__argc != 2 || __argv[1][1] != '\0' || !isdigit(__argv[1][0]))
        throw std::invalid_argument("The first command line argument must be a digit");

    return static_cast<userdata_kind>(__argv[1][0] - '0');
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        userdata_init(parse_command_line());

        constexpr grid_info grid{10, 10, 10};
        constexpr layout_info layout{10, 20, 20};
        const HWND window = create_window(to_SIZE(layout_grid_size(layout, grid)));
        g_labels = create_labels(window, layout, grid);
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