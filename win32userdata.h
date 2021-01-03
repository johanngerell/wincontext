#pragma once

#include <windows.h>
#include <memory>

class userdata
{
public:
    template <typename T>
    void set(HWND hwnd, T* data)
    {
        set_impl(hwnd, data);
    }

    template <typename T>
    T& get(HWND hwnd)
    {
        return *reinterpret_cast<T*>(get_impl(hwnd));
    }

    template <typename T>
    T* try_get(HWND hwnd)
    {
        return reinterpret_cast<T*>(get_impl(hwnd));
    }

    virtual const char* description() = 0;

//protected:
    virtual void set_impl(HWND hwnd, void* data) = 0;
    virtual void* get_impl(HWND hwnd) = 0;

    virtual ~userdata() = default;
};

std::unique_ptr<userdata> make_userdata(int impl_index, int size_hint);