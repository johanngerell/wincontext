#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <iterator>
#include "win32userdata.h"
#include "win32api.h"

std::unique_ptr<userdata> g_main_window_userdata;
std::unique_ptr<userdata> g_label_userdata;

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

std::string benchmark_userdata_access(const std::vector<HWND>& labels)
{
    const auto average_us = benchmark_average_us(100, [&labels]
    {
        for (const HWND label : labels)
            g_label_userdata->get<int>(label) += 1;
    });

    std::string text("average time: ");
    text += std::to_string(average_us);
    text += " us (";
    text += g_label_userdata->description();
    text += ")";

    return text;
}

using message_map = std::unordered_map<UINT, std::function<LRESULT(HWND, WPARAM, LPARAM)>>;

LRESULT CALLBACK message_map_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (auto handlers = g_main_window_userdata->try_get<message_map>(hwnd))
        if (auto it = handlers->find(msg); it != handlers->end())
            return it->second(hwnd, wp, lp);

    return DefWindowProc(hwnd, msg, wp, lp);
}

HWND create_main_window(const grid_cell_layout& cell_layout, const grid_dimensions& grid)
{
    window_creation_info creation_info;
    creation_info.class_name = register_window_class(message_map_wndproc, "Main Window Class");
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

void handle_messages(message_map& map, const std::vector<HWND>& labels, int impl_index)
{
    map[WM_CHAR] = [&labels, impl_index] (HWND hwnd, WPARAM wp, LPARAM) 
    {
        switch (static_cast<char>(wp))
        {
            case 'g': SetWindowTextA(hwnd, benchmark_userdata_access(labels).c_str()); break;
            case 'q': DestroyWindow(hwnd); break;
        }
        return 0;
    };

    map[WM_DESTROY] = [] (HWND, WPARAM, LPARAM)
    {
        PostQuitMessage(0);
        return 0;
    };
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    constexpr grid_dimensions grid{10, 10, 10};
    constexpr grid_cell_layout cell_layout{10, {20, 20}};

    if (__argc < 2 || __argv[1][1] != '\0' || !isdigit(__argv[1][0]))
        return 1;

    const int impl_index = __argv[1][0] - '0';
    g_label_userdata = make_userdata(impl_index, grid.row_count * grid.column_count * grid.layer_count);
    g_main_window_userdata = make_userdata(1, 0);

    const HWND main_window = create_main_window(cell_layout, grid);
    message_map main_message_map;
    g_main_window_userdata->set(main_window, &main_message_map);

    const std::vector<HWND> labels = create_labels(main_window, cell_layout, grid);
    std::vector<int> labels_userdata(labels.size());
    for (int i = 0; i < labels.size(); ++i)
    {
        labels_userdata[i] = rand() % 4711;
        g_label_userdata->set(labels[i], &labels_userdata[i]);
    }

    handle_messages(main_message_map, labels, impl_index);
    simple_message_loop();

    return 0;
}