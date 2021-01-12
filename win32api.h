#pragma once

#include "win32error.h"

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
