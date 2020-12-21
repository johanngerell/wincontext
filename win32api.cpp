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

HWND create_window(const window_info& info)
{
    const HWND hwnd = CreateWindowA(info.class_name, info.text, info.style | WS_VISIBLE | (info.parent ? WS_CHILD : 0),
                                    info.position.x, info.position.y, info.size.cx, info.size.cy,
                                    info.parent, nullptr, GetModuleHandle(nullptr), nullptr);

    if (!hwnd)
        throw win32_error{GetLastError(), "CreateWindowA"};
    
    return hwnd;
}

SIZE window_size_for_client(SIZE client_size, DWORD window_style)
{
    RECT bounds{0, 0, client_size.cx, client_size.cy};
    if (AdjustWindowRect(&bounds, window_style, FALSE) == 0)
        throw win32_error{GetLastError(), "AdjustWindowRect"};

    return {bounds.right - bounds.left, bounds.bottom - bounds.top};
}

void simple_message_loop()
{
    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
