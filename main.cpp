#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
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

template <typename Func>
std::chrono::microseconds::rep benchmark_average_us(int sample_count, Func&& func)
{
    auto t1 = std::chrono::high_resolution_clock::now();

    for (int sample = 0; sample < sample_count; ++sample)
        func();

    auto t2 = std::chrono::high_resolution_clock::now();                
    
    return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / sample_count;
}

using message_handler_void = std::function<void(WPARAM, LPARAM)>;
using message_handler_lresult = std::function<LRESULT(WPARAM, LPARAM)>;
using message_map = std::unordered_map<UINT, message_handler_lresult>;

void on_message(message_map& map, UINT msg, message_handler_void func, LRESULT result = 0)
{
    map[msg] = [func = std::move(func), result] (WPARAM wp, LPARAM lp)
    {
        func(wp, lp);
        return result;
    };
}

void on_message_ex(message_map& map, UINT msg, message_handler_lresult func)
{
    map[msg] = std::move(func);
}

LRESULT CALLBACK message_map_window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (auto map = try_get_userdata<message_map>(hwnd))
        if (auto it = map->find(msg); it != map->end())
            return it->second(wp, lp);

    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // Main window
    window_info popup_create_info;
    popup_create_info.class_name = register_window_class(message_map_window_proc, "Window Context Test Class");
    popup_create_info.text       = "Press 'g' to measure userdata access";
    popup_create_info.style      = WS_POPUPWINDOW | WS_CAPTION;
    popup_create_info.position   = {100, 100};
    popup_create_info.size       = window_size_for_client(client_size(), popup_create_info.style);
    const HWND popup             = create_window(popup_create_info);

    message_map popup_map;
    set_userdata(popup, &popup_map);

    // Child static controls, set position in each iteration below.
    window_info label_create_info;
    label_create_info.parent     = popup;
    label_create_info.class_name = "STATIC";
    label_create_info.style      = SS_BLACKFRAME;
    label_create_info.size       = cell_size;

    std::vector<HWND> labels(label_count);
    std::vector<int> label_values(label_count);

    for (int i = 0; i < label_count; ++i)
    {
        const auto location = grid_location_from_index(i);
        label_create_info.position = position_from_grid_location(location);
        labels[i] = create_window(label_create_info);
        label_values[i] = rand() % 4711;
        set_userdata(labels[i], &label_values[i]);
    }

    auto benchmark_userdata_access = [&labels, popup]
    {
        const auto average_us = benchmark_average_us(100, [&labels]
        {
            for (const HWND label : labels)
                get_userdata<int>(label) += 1;
        });

        std::string text("average time: ");
        text += std::to_string(average_us);
        text += " us (";
        text += get_userdata_description();
        text += ")";

        SetWindowTextA(popup, text.c_str());
    };

    on_message(popup_map, WM_CHAR, [=] (WPARAM wp, LPARAM) 
    {
        if (static_cast<char>(wp) == 'g')
            benchmark_userdata_access();
    });

    on_message(popup_map, WM_DESTROY, [] (WPARAM, LPARAM)
    {
        PostQuitMessage(0);
    });

    simple_message_loop();

    return 1;
}