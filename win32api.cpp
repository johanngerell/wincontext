#include "win32api.h"

namespace
{

// COLOR_WINDOW, COLOR_BACKGROUND, etc.
HBRUSH system_color_brush(int color_index)
{
    const HBRUSH hbrush = GetSysColorBrush(color_index);

    if (!hbrush)
        throw win32_error{"GetSysColorBrush"};
    
    return hbrush;
}

// IDC_ARROW, IDC_HAND, etc.
HCURSOR system_cursor(const char* resource_name)
{
    const HCURSOR hcursor = LoadCursorA(nullptr, resource_name);

    if (!hcursor)
        throw win32_error{"LoadCursorA"};
    
    return hcursor;
}

#ifdef OEMRESOURCE
// OCR_NORMAL, OCR_HAND, etc.
HCURSOR system_cursor(int resource_id)
{
    const HANDLE hcursor = LoadImageA(nullptr, MAKEINTRESOURCEA(resource_id),
                                      IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);

    if (!hcursor)
        throw win32_error{"LoadImageA"};
    
    return static_cast<HCURSOR>(hcursor);
}
#endif

}

const char* register_window_class(WNDPROC wndproc, const char* name)
{
    WNDCLASSA wc{};
    wc.lpfnWndProc   = wndproc;
    wc.lpszClassName = name;
    wc.hInstance     = GetModuleHandle(nullptr);
    wc.hbrBackground = system_color_brush(COLOR_WINDOW);
    wc.hCursor       = system_cursor(IDC_ARROW);

    if (RegisterClassA(&wc) == 0 && GetLastError() != ERROR_ALREADY_EXISTS)
        throw win32_error{"RegisterClassA"};

    return name;
}

HWND create_window(const window_creation_info& info)
{
    const HWND hwnd{CreateWindowA(info.class_name, info.text, info.style | WS_VISIBLE | (info.parent ? WS_CHILD : 0),
                                  info.position.x, info.position.y, info.size.cx, info.size.cy,
                                  info.parent, nullptr, GetModuleHandle(nullptr), nullptr)};

    if (!hwnd)
        throw win32_error{"CreateWindowA"};
    
    return hwnd;
}

SIZE window_size_for_client(SIZE client_size, DWORD window_style)
{
    RECT bounds{0, 0, client_size.cx, client_size.cy};

    if (AdjustWindowRect(&bounds, window_style, FALSE) == 0)
        throw win32_error{"AdjustWindowRect"};

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
