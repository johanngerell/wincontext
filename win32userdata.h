#pragma once

#include <windows.h>

const char* get_userdata_description();

void set_userdata_impl(HWND hwnd, void* userdata);
void* get_userdata_impl(HWND hwnd);

template <typename T>
void set_userdata(HWND hwnd, T* userdata)
{
    set_userdata_impl(hwnd, userdata);
}

inline void clear_userdata(HWND hwnd)
{
    set_userdata_impl(hwnd, nullptr);
}

template <typename T>
T& get_userdata(HWND hwnd)
{
    return *reinterpret_cast<T*>(get_userdata_impl(hwnd));
}

template <typename T>
T* try_get_userdata(HWND hwnd)
{
    return reinterpret_cast<T*>(get_userdata_impl(hwnd));
}
