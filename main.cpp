#include <algorithm>
#include <string>
#include <vector>
#include "jg_benchmark.h"
#include "win32api.h"
#include "app_options.h"

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

HWND create_window(layout_size client_size)
{
    window_creation_info creation_info;
    creation_info.class_name = register_window_class(main_wndproc, "Main Window Class");
    creation_info.text       = "Press 'g' to measure userdata access";
    creation_info.style      = WS_POPUPWINDOW | WS_CAPTION;
    creation_info.size       = window_size_for_client(to_SIZE(client_size), creation_info.style);
    creation_info.position   = {100, 100};

    return create_window(creation_info);
}

HWND create_label(HWND parent, layout_point point, layout_size size)
{
    window_creation_info creation_info;
    creation_info.parent     = parent;
    creation_info.class_name = "STATIC";
    creation_info.style      = SS_BLACKFRAME;
    creation_info.size       = to_SIZE(size);
    creation_info.position   = to_POINT(point);

    return create_window(creation_info);
}

std::vector<HWND> create_labels(HWND parent, const layout_info& layout, const grid_info& grid)
{
    std::vector<HWND> labels(grid.row_count * grid.column_count * grid.layer_count);

    std::generate(labels.begin(), labels.end(), [&, i = 0] () mutable
    {
        const auto cell = grid_cell_at(grid, i++);
        return create_label(parent, layout_cell_point(layout, cell), layout.cell_size);
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

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        app_options options{args{__argc, __argv}};
        userdata_init(options.kind);
        const HWND window = create_window(layout_grid_size(options.layout, options.grid));
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