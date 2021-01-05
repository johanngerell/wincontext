#pragma once

#include <windows.h>

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

void userdata_init(userdata_kind kind);
const char* userdata_description();
void userdata_set(HWND hwnd, void* data);
void* userdata_get(HWND hwnd);
