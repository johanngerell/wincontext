#include <chrono>
#include <string>
#include <vector>
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

HWND create_window(const grid_cell_layout& cell_layout, const grid_dimensions& grid)
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

    const int userdata_kind_index = __argv[1][0] - '0';
    userdata_init(static_cast<userdata_kind>(userdata_kind_index));

    const HWND window = create_window(cell_layout, grid);
    g_labels = create_labels(window, cell_layout, grid);
    g_labels_data.reserve(g_labels.size());

    for (const HWND label : g_labels)
        userdata_set(label, &g_labels_data.emplace_back(rand() % 4711));

    simple_message_loop();

    return 0;
}