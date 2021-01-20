#pragma once

#include <windows.h>
#include <memory>

enum class userdata_kind
{
    baseline,
    get_set_window_long,
    get_set_prop,
    get_set_prop_atom,
    unordered_map,
    map,
    vector_sorted,
    vector_unsorted
};

class userdata
{
public:
    virtual void set(HWND hwnd, void* data) = 0;
    virtual void* get(HWND hwnd) = 0;
    virtual const char* description() = 0;

    virtual ~userdata() = default;
};

std::unique_ptr<userdata> create_userdata(userdata_kind kind);
