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

    bool operator==(const grid_location& other) const
    {
        return row == other.row && column == other.column && layer == other.layer;
    }

    bool operator!=(const grid_location& other) const
    {
        return !(*this == other);
    }
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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_CHAR:
        {
            const char c = static_cast<char>(wp);

            if (c == 'g')
            {
                const auto& labels = get_userdata<std::vector<HWND>>(hwnd);

                auto t1 = std::chrono::high_resolution_clock::now();

                for (int i = 0; i < static_cast<int>(labels.size()); ++i)
                {
                    const auto location1 = grid_location_from_index(i);
                    const auto location2 = get_userdata<grid_location>(labels[i]);

                    if (location1 != location2)
                        throw std::logic_error("location mismatch");
                }

                auto t2 = std::chrono::high_resolution_clock::now();
                auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);

                std::string time_msg;
                time_msg += "Time: ";
                time_msg += std::to_string(duration_us.count());
                time_msg += " us";

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
    label_create_info.position   = {}; // gets set at each iteration below.
    label_create_info.size       = cell_size;

    std::vector<HWND> labels;
    labels.reserve(label_count);

    std::vector<grid_location> locations;
    locations.reserve(label_count);

    for (int i = 0; i < label_count; ++i)
    {
        locations.push_back(grid_location_from_index(i));
        label_create_info.position = position_from_grid_location(locations.back());
        labels.push_back(create_window(label_create_info));
        set_userdata(labels.back(), &locations.back());
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