#pragma once

#include <windows.h>

const char* get_userdata_description(int impl_index);

void set_userdata_impl(int impl_index, int hwnd_index, HWND hwnd, void* userdata);
void* get_userdata_impl(int impl_index, int hwnd_index, HWND hwnd);

template <typename T>
void set_userdata(int impl_index, int hwnd_index, HWND hwnd, T* userdata)
{
    set_userdata_impl(impl_index, hwnd_index, hwnd, userdata);
}

template <typename T>
T& get_userdata(int impl_index, int hwnd_index, HWND hwnd)
{
    return *reinterpret_cast<T*>(get_userdata_impl(impl_index, hwnd_index, hwnd));
}

template <typename T>
T* try_get_userdata(int impl_index, int hwnd_index, HWND hwnd)
{
    return reinterpret_cast<T*>(get_userdata_impl(impl_index, hwnd_index, hwnd));
}
