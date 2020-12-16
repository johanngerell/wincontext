#include <windows.h>
#include <system_error>
#include <unordered_map>
#include <map>
#include <chrono>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

class win32_error : std::runtime_error
{
public:
    explicit win32_error(DWORD code, const char* function)
        : std::runtime_error(function)
        , m_code{code}
    {}

    DWORD code() const { return m_code; }

private:
    DWORD m_code{};
};

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

#define METHOD1

#if defined(METHOD22)
    ATOM g_userdata_atom = GlobalAddAtomA("userdata");
#endif

#if defined(METHOD3)
    std::unordered_map<HWND, void*> g_userdata;
#endif

#if defined(METHOD4)
    std::map<HWND, void*> g_userdata;
#endif

#if defined(METHOD5)
    struct userdata_item
    {
        HWND hwnd;
        void* data;
    };

    bool operator<(const userdata_item& first, const userdata_item& second)
    {
        return first.hwnd < second.hwnd;
    }

    std::vector<userdata_item> g_userdata;
#endif

void set_userdata_impl(HWND hwnd, void* userdata)
{
#if defined(METHOD1)
    SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userdata));
#elif defined(METHOD2)
    SetPropA(hwnd, "userdata", userdata);
#elif defined(METHOD22)
    SetPropA(hwnd, reinterpret_cast<const char*>(MAKELONG(g_userdata_atom, 0)), userdata);
#elif defined(METHOD3)
    g_userdata[hwnd] = userdata;
#elif defined(METHOD4)
    g_userdata[hwnd] = userdata;
#elif defined(METHOD5)
    userdata_item item{hwnd, userdata};
    g_userdata.insert(std::upper_bound(g_userdata.begin(), g_userdata.end(), item), item);
#endif
}

template <typename T>
void set_userdata(HWND hwnd, T* userdata)
{
    set_userdata_impl(hwnd, userdata);
}

void* get_userdata_impl(HWND hwnd)
{
#if defined(METHOD1)
    return reinterpret_cast<void*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
#elif defined(METHOD2)
    return reinterpret_cast<void*>(GetPropA(hwnd, "userdata"));
#elif defined(METHOD22)
    return reinterpret_cast<void*>(GetPropA(hwnd, reinterpret_cast<const char*>(MAKELONG(g_userdata_atom, 0))));
#elif defined(METHOD3)
    auto it = g_userdata.find(hwnd);
    if (it != g_userdata.end())
        return it->second;
    else
        return nullptr;
#elif defined(METHOD4)
    auto it = g_userdata.find(hwnd);
    if (it != g_userdata.end())
        return it->second;
    else
        return nullptr;
#elif defined(METHOD5)
    userdata_item item{hwnd, nullptr};
    auto it = std::lower_bound(g_userdata.begin(), g_userdata.end(), item);
    if (it != g_userdata.end() && it->hwnd == hwnd)
        return it->data;
    else
        return nullptr;
#endif
}

template <typename T>
T* get_userdata(HWND hwnd)
{
    return reinterpret_cast<T*>(get_userdata_impl(hwnd));
}

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
                auto duration_ns = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);

                std::string msg = "Done: ";
                msg += std::to_string(duration_ns.count());
                msg += " us";

                MessageBoxA(hwnd, msg.c_str(), "Done", MB_OK);
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
constexpr int col_count{10};
constexpr int layer_count{10};
constexpr int cell_spacing{10};
constexpr SIZE cell_size{20, 20};
constexpr SIZE client_size
{
    col_count * (cell_size.cx + cell_spacing) + cell_spacing + layer_count * 2,
    row_count * (cell_size.cy + cell_spacing) + cell_spacing + layer_count * 2
};

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
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