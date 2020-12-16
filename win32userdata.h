#include <windows.h>

void set_userdata_impl(HWND hwnd, void* userdata);
void* get_userdata_impl(HWND hwnd);

template <typename T>
void set_userdata(HWND hwnd, T* userdata)
{
    set_userdata_impl(hwnd, userdata);
}

template <typename T>
T* get_userdata(HWND hwnd)
{
    return reinterpret_cast<T*>(get_userdata_impl(hwnd));
}
