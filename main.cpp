#include <chrono>
#include <string>
#include <vector>
#include "win32userdata.h"
#include "win32api.h"

constexpr int row_count{10};
constexpr int column_count{10};
constexpr int layer_count{10};
constexpr int label_count{row_count * column_count * layer_count};

struct grid_location
{
    int row{};
    int column{};
    int layer{};
};

grid_location grid_location_from_index(int index)
{
    return
    {
        (index % (row_count * column_count)) / column_count,
        (index % (row_count * column_count)) % column_count,
        index / (row_count * column_count)
    };
}

constexpr int cell_spacing{10};
constexpr SIZE cell_size{20, 20};

SIZE client_size()
{
    return
    {
        column_count * (cell_size.cx + cell_spacing) + cell_spacing + layer_count * 2,
        row_count * (cell_size.cy + cell_spacing) + cell_spacing + layer_count * 2
    };
};

POINT position_from_grid_location(const grid_location& location)
{
    return
    {
        cell_spacing + cell_size.cy * location.row + cell_spacing * location.row + location.layer * 2,
        cell_spacing + cell_size.cx * location.column + cell_spacing * location.column + location.layer * 2
    };
}

struct label_info
{
    HWND hwnd{};
    int value{};
};

template <typename Func>
std::chrono::microseconds::rep benchmark(int sample_count, Func&& func)
{
    auto t1 = std::chrono::high_resolution_clock::now();

    for (int sample = 0; sample < sample_count; ++sample)
        func();
    
    auto t2 = std::chrono::high_resolution_clock::now();                
    
    return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / sample_count;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_CHAR:
        {
            const char c = static_cast<char>(wp);

            if (c == 'g')
            {
                const auto& labels = get_userdata<std::vector<label_info>>(hwnd);
                bool success = true;

                const auto average_us = benchmark(10, [&labels, &success]
                {
                    for (const auto& label : labels)
                    {
                        auto& userdata_value = get_userdata<int>(label.hwnd);

                        if (userdata_value != label.value)
                            success = false;
        
                        if (++userdata_value != label.value)
                            success = false;
                    }
                });

                std::string time_msg;
                time_msg += "Average time: ";
                time_msg += std::to_string(average_us);
                time_msg += success ? " us (no errors)" : " us (with errors)";

                SetWindowTextA(hwnd, time_msg.c_str());
            }

            return 0;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    window_info popup_create_info;
    popup_create_info.class_name = register_window_class(WindowProc, "Window Context Test Class");
    popup_create_info.text       = "Window Context Test";
    popup_create_info.style      = WS_POPUPWINDOW | WS_CAPTION;
    popup_create_info.position   = {100, 100};
    popup_create_info.size       = window_size_for_client(client_size(), popup_create_info.style);
    const HWND popup             = create_window(popup_create_info);

    window_info label_create_info;
    label_create_info.parent     = popup;
    label_create_info.class_name = "STATIC";
    label_create_info.style      = SS_BLACKFRAME | SS_NOTIFY;
    label_create_info.position   = {}; // set at each iteration below.
    label_create_info.size       = cell_size;

    std::vector<label_info> labels;
    labels.reserve(label_count);

    for (int i = 0; i < label_count; ++i)
    {
        const auto location = grid_location_from_index(i);
        label_create_info.position = position_from_grid_location(location);
        const HWND label = create_window(label_create_info);
        labels.push_back({label, rand() % 4711});
        set_userdata(label, &labels.back().value);
    }

    set_userdata(popup, &labels);

    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 1;
}