#include "win32_api.h"
#include "win32_error.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <iostream>
#endif

#ifdef _WIN32

namespace
{

// COLOR_WINDOW, COLOR_BACKGROUND, etc.
HBRUSH system_color_brush(int color_index)
{
    const HBRUSH hbrush = GetSysColorBrush(color_index);

    if (!hbrush)
        throw win32_error{"GetSysColorBrush", GetLastError()};
    
    return hbrush;
}

// IDC_ARROW, IDC_HAND, etc.
HCURSOR system_cursor(const char* resource_name)
{
    const HCURSOR hcursor = LoadCursorA(nullptr, resource_name);

    if (!hcursor)
        throw win32_error{"LoadCursorA", GetLastError()};
    
    return hcursor;
}

#ifdef OEMRESOURCE
// OCR_NORMAL, OCR_HAND, etc.
HCURSOR system_cursor(int resource_id)
{
    const HANDLE hcursor = LoadImageA(nullptr, MAKEINTRESOURCEA(resource_id),
                                      IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);

    if (!hcursor)
        throw win32_error{"LoadImageA", GetLastError()};
    
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
        throw win32_error{"RegisterClassA", GetLastError()};

    return name;
}

HWND create_window(const window_creation_info& info)
{
    const HWND hwnd{CreateWindowA(info.class_name, info.text, info.style | WS_VISIBLE | (info.parent ? WS_CHILD : 0),
                                  info.position.x, info.position.y, info.size.cx, info.size.cy,
                                  info.parent, nullptr, GetModuleHandle(nullptr), nullptr)};

    if (!hwnd)
        throw win32_error{"CreateWindowA", GetLastError()};
    
    return hwnd;
}

SIZE window_size_for_client(SIZE client_size, DWORD window_style)
{
    RECT bounds{0, 0, client_size.cx, client_size.cy};

    if (AdjustWindowRect(&bounds, window_style, FALSE) == 0)
        throw win32_error{"AdjustWindowRect", GetLastError()};

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

void message_box(HWND owner, const char* title, const char* text, UINT style)
{
    MessageBoxA(owner, text, title, style);
}

LRESULT def_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    return DefWindowProc(hwnd, msg, wp, lp);
}

void post_quit_message(int exit_code)
{
    PostQuitMessage(exit_code);
}

void set_window_text(HWND hwnd, const char* text)
{
    SetWindowTextA(hwnd, text);
}

void destroy_window(HWND hwnd)
{
    DestroyWindow(hwnd);
}

#else

const char* register_window_class(WNDPROC, const char* name)
{
    return name;
}

HWND create_window(const window_creation_info&)
{
    return reinterpret_cast<HWND>(4711);
}

SIZE window_size_for_client(SIZE client_size, DWORD)
{
    return client_size;
}

void simple_message_loop()
{
}

void message_box(HWND, const char* title, const char* text, UINT)
{
    std::cout << title << '\n';
    std::cout << text << '\n';
}

LRESULT def_wndproc(HWND, UINT, WPARAM, LPARAM)
{
    return 0;
}

void post_quit_message(int)
{
}

void set_window_text(HWND, const char*)
{
}

void destroy_window(HWND)
{
}

#endif