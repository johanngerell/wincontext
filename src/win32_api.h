#pragma once

#ifndef _WIN32
    #define CALLBACK
    using DWORD = unsigned long;
    using LONG = signed long;
    using HWND = void*;
    using UINT = unsigned int;
    using WPARAM = unsigned long;
    using LPARAM = signed long;
    using LRESULT = unsigned long;
    using WNDPROC = LRESULT (CALLBACK*)(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    struct POINT { long x, y; };
    struct SIZE { long cx, cy; };
    enum { WM_CHAR = 0x0102, WM_DESTROY = 0x0002 };
    enum { WS_POPUP = 0x80000000L, WS_BORDER = 0x00800000L, WS_SYSMENU = 0x00080000L, WS_CAPTION = 0x00C00000L,
           WS_POPUPWINDOW = WS_POPUP | WS_BORDER | WS_SYSMENU,
           SS_BLACKFRAME = 0x7L };
    enum { MB_OK = 0x0L };
#endif

const char* register_window_class(WNDPROC wndproc, const char* name);

struct window_creation_info
{
    const char* class_name{};
    const char* text{};
    DWORD style{};
    POINT position{};
    SIZE size{};
    HWND parent{};
};

HWND create_window(const window_creation_info& info);

SIZE window_size_for_client(SIZE client_size, DWORD window_style);

void simple_message_loop();

void message_box(HWND owner, const char* title, const char* text, UINT style);

LRESULT CALLBACK def_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

void post_quit_message(int exit_code);

void set_window_text(HWND hwnd, const char* text);

void destroy_window(HWND hwnd);
