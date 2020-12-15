#include <windows.h>
#include <system_error>

struct win32_error
{
    DWORD code{};
    const char* function{};
};

struct cell_data
{
    int row{};
    int col{};
    HWND hwnd{nullptr};
    bool visited{};
};

struct sheet_data
{
    cell_data* cells{};
    int cell_count{};
};

void set_userdata(HWND hwnd, void* userdata)
{
    SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userdata));
}

template <typename T>
T& get_userdata(HWND hwnd)
{
    return *reinterpret_cast<T*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_COMMAND:
        {
            if (HIWORD(wp) == STN_CLICKED)
            {
                sheet_data& sheet = get_userdata<sheet_data>(hwnd);

                for (int i = 0; i < sheet.cell_count; ++i)
                {
                    cell_data& cell = sheet.cells[i];
                    cell_data& userdata_cell = get_userdata<cell_data>(cell.hwnd);

                    if (&userdata_cell != &cell)
                        throw std::logic_error("userdata_cell != &cell");

                    if (cell.visited)
                        throw std::logic_error("cell.visited");

                    cell.visited = true;
                }

                MessageBoxA(hwnd, "Done", "Looping", MB_OK);
            }
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}

const char* register_window_class(WNDPROC wndproc, const char* name)
{
    WNDCLASSA wc{};
    wc.lpfnWndProc   = wndproc;
    wc.hInstance     = GetModuleHandle(nullptr);
    wc.lpszClassName = name;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    
    if (RegisterClassA(&wc) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
        throw win32_error{GetLastError(), "RegisterClassA"};

    return name;
}

HWND create_window(HWND parent, const char* class_name, const char* text, DWORD style,
                   int x, int y, int width, int height, void* context = nullptr)
{
    const HWND hwnd = CreateWindowA(class_name, text, style | WS_VISIBLE | (parent ? WS_CHILD : 0),
                                    x, y, width, height, parent, nullptr, GetModuleHandle(nullptr), context);

    if (!hwnd)
        throw win32_error{GetLastError(), "CreateWindowA"};
    
    return hwnd;
}

SIZE window_size_for_client(int width, int height, DWORD style)
{
    RECT bounds{0, 0, width, height};
    if (AdjustWindowRect(&bounds, style, FALSE) == 0)
        throw win32_error{GetLastError(), "AdjustWindowRect"};

    return {bounds.right - bounds.left, bounds.bottom - bounds.top};
}

constexpr int row_count{10};
constexpr int column_count{10};
constexpr int cell_spacing{10};
constexpr SIZE cell_size{20, 20};
constexpr SIZE client_size
{
    column_count * (cell_size.cx + cell_spacing) + cell_spacing,
    row_count    * (cell_size.cy + cell_spacing) + cell_spacing
};

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    const char* popup_class_name = register_window_class(WindowProc, "Window Context Test Class");
    const DWORD popup_style = WS_POPUPWINDOW | WS_CAPTION;
    const SIZE popup_size = window_size_for_client(client_size.cx, client_size.cy, popup_style);
    const HWND popup = create_window(nullptr, popup_class_name, "Window Context Test", popup_style,
                                     100, 100, popup_size.cx, popup_size.cy);

    cell_data cells[row_count * column_count]{};
    for (int row = 0; row < row_count; ++row)
        for (int col = 0; col < column_count; ++col)
        {
            cell_data& data = cells[row * column_count + col];
            data.row = row;
            data.col = col;
            const HWND cell = create_window(popup, "STATIC", "", SS_BLACKRECT | SS_NOTIFY,
                                            cell_spacing + cell_size.cy * row + cell_spacing * row,
                                            cell_spacing + cell_size.cx * col + cell_spacing * col,
                                            cell_size.cx, cell_size.cy);
            data.hwnd = cell;
            set_userdata(cell, &data);
        }

    sheet_data sheet{cells, row_count * column_count};
    set_userdata(popup, &sheet);

    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 1;
}