#include <chrono>
#include <string>
#include "userdata.h"
#include "win32api.h"

struct cell_data
{
    int row{};
    int col{};
    int layer{};
    HWND hwnd{nullptr};
    bool visited{};
};

struct sheet_data
{
    cell_data* cells{};
    int cell_count{};
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_COMMAND:
        {
            if (HIWORD(wp) == STN_CLICKED)
            {
                sheet_data& sheet = *get_userdata<sheet_data>(hwnd);

                auto t1 = std::chrono::high_resolution_clock::now();

                int i = 0;

                for (; i < sheet.cell_count; ++i)
                {
                    cell_data& cell = sheet.cells[i];
                    cell_data& userdata_cell = *get_userdata<cell_data>(cell.hwnd);

                    if (&userdata_cell != &cell)
                        throw std::logic_error("userdata_cell != &cell");

                    if (cell.visited)
                        throw std::logic_error("cell.visited");

                    cell.visited = true;
                }

                if (i != sheet.cell_count)
                    throw std::logic_error("i != sheet.cell_count");

                auto t2 = std::chrono::high_resolution_clock::now();
                auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);

                std::string time_msg;
                time_msg += "Time: ";
                time_msg += std::to_string(duration_us.count());
                time_msg += " us";

                MessageBoxA(hwnd, time_msg.c_str(), "Done", MB_OK);
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

constexpr int row_count{10};
constexpr int col_count{10};
constexpr int layer_count{10};
constexpr int cell_spacing{10};
constexpr SIZE cell_size{20, 20};
constexpr SIZE client_size
{
    col_count * (cell_size.cx + cell_spacing) + cell_spacing + layer_count * 2,
    row_count * (cell_size.cy + cell_spacing) + cell_spacing + layer_count * 2
};

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    const char* popup_class_name = register_window_class(WindowProc, "Window Context Test Class");
    const DWORD popup_style = WS_POPUPWINDOW | WS_CAPTION;
    const SIZE popup_size = window_size_for_client(client_size.cx, client_size.cy, popup_style);
    const HWND popup = create_window(nullptr, popup_class_name, "Window Context Test", popup_style,
                                     100, 100, popup_size.cx, popup_size.cy);

    cell_data cells[row_count * col_count * layer_count]{};
    for (int layer = 0; layer < layer_count; ++layer)
    {
        for (int row = 0; row < row_count; ++row)
        {
            int offset = layer * row_count * col_count + row * col_count;
            for (int col = 0; col < col_count; ++col)
            {
                cell_data& data = cells[offset + col];
                data.row = row;
                data.col = col;
                const HWND cell = create_window(popup, "STATIC", "", SS_BLACKFRAME | SS_NOTIFY,
                                                cell_spacing + cell_size.cy * row + cell_spacing * row + layer * 2,
                                                cell_spacing + cell_size.cx * col + cell_spacing * col + layer * 2,
                                                cell_size.cx, cell_size.cy);
                data.hwnd = cell;
                set_userdata(cell, &data);
            }
        }
    }

    sheet_data sheet{cells, row_count * col_count * layer_count};
    set_userdata(popup, &sheet);

    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 1;
}