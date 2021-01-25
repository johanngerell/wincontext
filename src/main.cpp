#include <algorithm>
#include <string>
#include <vector>
#include <jg_benchmark.h>
#include "win32_api.h"
#include "app_options.h"

std::unique_ptr<userdata> g_userdata;
HWND g_main_window;
std::vector<HWND> g_labels;
std::vector<int> g_data;

std::string benchmark_userdata_access()
{
    constexpr size_t sample_count = 100;
    const std::vector<int> old_data = g_data;

    auto add_1 = [] (HWND label) { *static_cast<int*>(g_userdata->get(label)) += 1; };
    const auto sample_ns = jg::benchmark(sample_count, [&] { for(HWND label : g_labels) add_1(label); });

    auto added_sample_count = [sample_count](int v1, int v2) { return v1 + sample_count == v2; };
    const bool mismatch = !std::equal(old_data.begin(), old_data.end(), g_data.begin(), g_data.end(), added_sample_count);
    
    if (mismatch)
        throw std::logic_error("Benchmark data mismatch");

    std::string text("average call time: ");
    text += std::to_string(sample_ns.count() / g_labels.size());
    text += " ns (";
    text += g_userdata->description();
    text += ")";

    return text;
}

constexpr char wm_char_key(WPARAM wp, LPARAM)
{
    return static_cast<char>(wp);
}

LRESULT CALLBACK main_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_CHAR:
            switch (wm_char_key(wp, lp))
            {
                case 'g': set_window_text(hwnd, benchmark_userdata_access().c_str()); break;
                case 'q': destroy_window(hwnd); break;
            }
            return 0;

        case WM_DESTROY:
            post_quit_message(0);
            return 0;

        default:
            return def_wndproc(hwnd, msg, wp, lp);
    }
}

void create_main_window(const layout_info& layout, const grid_info& grid)
{
    const SIZE size{to_SIZE(layout_grid_size(layout, grid))};

    window_creation_info creation_info;
    creation_info.class_name = register_window_class(main_wndproc, "Main Window Class");
    creation_info.text       = "Press 'g' to measure userdata access";
    creation_info.style      = WS_POPUPWINDOW | WS_CAPTION;
    creation_info.size       = window_size_for_client(size, creation_info.style);
    creation_info.position   = {100, 100};

    g_main_window = create_window(creation_info);
}

void create_labels(const layout_info& layout, const grid_info& grid)
{
    auto create_label = [] (layout_point point, layout_size size)
    {
        window_creation_info creation_info;
        creation_info.parent     = g_main_window;
        creation_info.class_name = "STATIC";
        creation_info.style      = SS_BLACKFRAME;
        creation_info.size       = to_SIZE(size);
        creation_info.position   = to_POINT(point);

        return create_window(creation_info);
    };

    g_labels.resize(grid.row_count * grid.column_count * grid.layer_count);

    std::generate(g_labels.begin(), g_labels.end(), [&, i = 0] () mutable
    {
        const auto cell = grid_cell_at(grid, i++);
        return create_label(layout_cell_point(layout, cell), layout.cell_size);
    });
}

void create_data()
{
    g_data.resize(g_labels.size());

    std::generate(g_data.begin(), g_data.end(), []
    {
        return rand() % 4711;
    });

    for (size_t i = 0; i < g_labels.size(); ++i)
        g_userdata->set(g_labels[i], &g_data[i]);
}

jg::args g_args;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#ifdef _WIN32
        g_args = {__argc, __argv};
#endif

    try
    {
        const app_options options{g_args};
        g_userdata = create_userdata(options.kind);
        create_main_window(options.layout, options.grid);
        create_labels(options.layout, options.grid);
        create_data();
 
        simple_message_loop();
    }
    catch(const std::exception& e)
    {
        message_box(nullptr, "Error", e.what(), MB_OK);
    }

    return 0;
}

#ifndef _WIN32
int main(int argc, char** argv)
{
    g_args = {argc, argv};
    return WinMain(nullptr, nullptr, nullptr, SW_SHOWNORMAL);
}
#endif
