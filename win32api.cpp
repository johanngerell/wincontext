#include "win32api.h"

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
                   int x, int y, int width, int height, void* context)
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