#include <chrono>
#include <string>
#include <vector>
#include "win32userdata.h"
#include "win32api.h"

struct grid_info 
{
    int row_count{};
    int column_count{};
    int layer_count{};
};

struct cell_info
{
    int row_index{};
    int column_index{};
    int layer_index{};
};

struct grid_layout
{
    int cell_spacing{};
    SIZE cell_size{};
};

cell_info cell_from_index(const grid_info& grid, int index)
{
    return
    {
        (index % (grid.row_count * grid.column_count)) / grid.column_count,
        (index % (grid.row_count * grid.column_count)) % grid.column_count,
         index / (grid.row_count * grid.column_count)
    };
}

SIZE grid_layout_size(const grid_info& grid, const grid_layout& layout)
{
    return
    {
        grid.column_count * (layout.cell_size.cx + layout.cell_spacing) + layout.cell_spacing + grid.layer_count * 2,
        grid.row_count    * (layout.cell_size.cy + layout.cell_spacing) + layout.cell_spacing + grid.layer_count * 2
    };
};

POINT cell_layout_position(const cell_info& cell, const grid_layout& layout)
{
    return
    {
        cell.row_index    * (layout.cell_size.cy + layout.cell_spacing) + layout.cell_spacing + cell.layer_index * 2,
        cell.column_index * (layout.cell_size.cx + layout.cell_spacing) + layout.cell_spacing + cell.layer_index * 2
    };
}

template <typename Func>
std::chrono::nanoseconds::rep benchmark_average_ns(int sample_count, Func&& func)
{
    const auto t1 = std::chrono::high_resolution_clock::now();

    for (int sample = 0; sample < sample_count; ++sample)
        func();

    const auto t2 = std::chrono::high_resolution_clock::now();
    
    return std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / sample_count;
}

std::vector<HWND> g_labels;
std::vector<int> g_labels_data;

std::string benchmark_label_userdata_access()
{
    constexpr int sample_count = 100;
    const std::vector<int> old_labels_data = g_labels_data;

    const auto average_sample_ns = benchmark_average_ns(sample_count, []
    {
        for (const HWND label : g_labels)
            *static_cast<int*>(userdata_get(label)) += 1;
    });

    for (int i = 0; i < g_labels.size(); ++i)
        if (old_labels_data[i] + sample_count != g_labels_data[i])
            throw std::logic_error("Data mismatch");

    std::string text("average call time: ");
    text += std::to_string(average_sample_ns / g_labels.size());
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

std::vector<HWND> create_labels(HWND parent, const grid_info& grid, const grid_layout& layout)
{
    window_creation_info creation_info;
    creation_info.parent     = parent;
    creation_info.class_name = "STATIC";
    creation_info.style      = SS_BLACKFRAME;
    creation_info.size       = layout.cell_size;

    // Layout all labels in a grid
    std::vector<HWND> labels(grid.row_count * grid.column_count * grid.layer_count);

    for (int i = 0; i < labels.size(); ++i)
    {
        const auto cell = cell_from_index(grid, i);
        creation_info.position = cell_layout_position(cell, layout);
        labels[i] = create_window(creation_info);
    }

    return labels;
}

userdata_kind parse_command_line()
{
    if (__argc < 2 || __argv[1][1] != '\0' || !isdigit(__argv[1][0]))
        throw std::invalid_argument("The first command line argument must be a digit");

    return static_cast<userdata_kind>(__argv[1][0] - '0');
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        userdata_init(parse_command_line());

        constexpr grid_info grid{10, 10, 10};
        constexpr grid_layout layout{10, {20, 20}};
        const HWND window = create_window(grid_layout_size(grid, layout));
        g_labels = create_labels(window, grid, layout);
        g_labels_data.reserve(g_labels.size());

        for (const HWND label : g_labels)
            userdata_set(label, &g_labels_data.emplace_back(rand() % 4711));

        simple_message_loop();
    }
    catch(const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Unhandled exception", MB_OK);
    }

    return 0;
}