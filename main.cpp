#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <iterator>
#include "win32userdata.h"
#include "win32api.h"

struct grid_dimensions
{
    int row_count{};
    int column_count{};
    int layer_count{};
};

struct grid_location
{
    int row_index{};
    int column_index{};
    int layer_index{};
};

grid_location grid_location_from_index(const grid_dimensions& grid, int index)
{
    return
    {
        (index % (grid.row_count * grid.column_count)) / grid.column_count,
        (index % (grid.row_count * grid.column_count)) % grid.column_count,
         index / (grid.row_count * grid.column_count)
    };
}

struct grid_cell_layout
{
    int spacing{};
    SIZE size{};
};

SIZE client_size_for_grid(const grid_dimensions& grid, const grid_cell_layout& layout)
{
    return
    {
        grid.column_count * (layout.size.cx + layout.spacing) + layout.spacing + grid.layer_count * 2,
        grid.row_count    * (layout.size.cy + layout.spacing) + layout.spacing + grid.layer_count * 2
    };
};

POINT position_from_grid_location(const grid_location& location, const grid_cell_layout& layout)
{
    return
    {
        layout.spacing + layout.size.cy * location.row_index    + layout.spacing * location.row_index    + location.layer_index * 2,
        layout.spacing + layout.size.cx * location.column_index + layout.spacing * location.column_index + location.layer_index * 2
    };
}

template <typename Func>
std::chrono::microseconds::rep benchmark_average_us(int sample_count, Func&& func)
{
    auto t1 = std::chrono::high_resolution_clock::now();

    for (int sample = 0; sample < sample_count; ++sample)
        func();

    auto t2 = std::chrono::high_resolution_clock::now();                
    
    return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / sample_count;
}

std::vector<HWND> g_labels;
std::unique_ptr<userdata> g_label_userdata;

std::string benchmark_label_userdata_access()
{
    const auto average_us = benchmark_average_us(100, []
    {
        for (const HWND label : g_labels)
            g_label_userdata->get<int>(label) += 1;
    });

    std::string text("average time: ");
    text += std::to_string(average_us);
    text += " us (";
    text += g_label_userdata->description();
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
                case 'g': SetWindowTextA(hwnd, benchmark_label_userdata_access().c_str()); break;
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

HWND create_main_window(const grid_cell_layout& cell_layout, const grid_dimensions& grid)
{
    window_creation_info creation_info;
    creation_info.class_name = register_window_class(main_wndproc, "Main Window Class");
    creation_info.text       = "Press 'g' to measure userdata access";
    creation_info.style      = WS_POPUPWINDOW | WS_CAPTION;
    creation_info.size       = window_size_for_client(client_size_for_grid(grid, cell_layout), creation_info.style);
    creation_info.position   = {100, 100};

    return create_window(creation_info);
}

std::vector<HWND> create_labels(HWND parent, const grid_cell_layout& cell_layout, const grid_dimensions& grid)
{
    window_creation_info creation_info;
    creation_info.parent     = parent;
    creation_info.class_name = "STATIC";
    creation_info.style      = SS_BLACKFRAME;
    creation_info.size       = cell_layout.size;

    // Layout all labels in a grid
    std::vector<HWND> labels(grid.row_count * grid.column_count * grid.layer_count);
    for (int i = 0; i < labels.size(); ++i)
    {
        const auto location = grid_location_from_index(grid, i);
        creation_info.position = position_from_grid_location(location, cell_layout);
        labels[i] = create_window(creation_info);
    }

    return labels;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    constexpr grid_dimensions grid{10, 10, 10};
    constexpr grid_cell_layout cell_layout{10, {20, 20}};

    if (__argc < 2 || __argv[1][1] != '\0' || !isdigit(__argv[1][0]))
        return 1;

    const int impl_index = __argv[1][0] - '0';
    g_label_userdata = make_userdata(impl_index, grid.row_count * grid.column_count * grid.layer_count);

    const HWND main_window = create_main_window(cell_layout, grid);

    g_labels = create_labels(main_window, cell_layout, grid);
    std::vector<int> labels_userdata(g_labels.size());
    for (int i = 0; i < g_labels.size(); ++i)
    {
        labels_userdata[i] = rand() % 4711;
        g_label_userdata->set(g_labels[i], &labels_userdata[i]);
    }

    simple_message_loop();

    return 0;
}