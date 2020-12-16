#include "win32userdata.h"

#define METHOD1

#if defined(METHOD22)
    ATOM g_userdata_atom = GlobalAddAtomA("userdata");
#endif

#if defined(METHOD3)
    #include <unordered_map>
    std::unordered_map<HWND, void*> g_userdata;
#endif

#if defined(METHOD4)
    #include <map>
    std::map<HWND, void*> g_userdata;
#endif

#if defined(METHOD5)
    #include <vector>
    #include <algorithm>

    struct userdata_item
    {
        HWND hwnd;
        void* data;
    };

    bool operator<(const userdata_item& first, const userdata_item& second)
    {
        return first.hwnd < second.hwnd;
    }

    std::vector<userdata_item> g_userdata;
#endif

void set_userdata_impl(HWND hwnd, void* userdata)
{
#if defined(METHOD1)
    SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userdata));
#elif defined(METHOD2)
    SetPropA(hwnd, "userdata", userdata);
#elif defined(METHOD22)
    SetPropA(hwnd, reinterpret_cast<const char*>(MAKELONG(g_userdata_atom, 0)), userdata);
#elif defined(METHOD3)
    g_userdata[hwnd] = userdata;
#elif defined(METHOD4)
    g_userdata[hwnd] = userdata;
#elif defined(METHOD5)
    userdata_item item{hwnd, userdata};
    g_userdata.insert(std::upper_bound(g_userdata.begin(), g_userdata.end(), item), item);
#endif
}

void* get_userdata_impl(HWND hwnd)
{
#if defined(METHOD1)
    return reinterpret_cast<void*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
#elif defined(METHOD2)
    return reinterpret_cast<void*>(GetPropA(hwnd, "userdata"));
#elif defined(METHOD22)
    return reinterpret_cast<void*>(GetPropA(hwnd, reinterpret_cast<const char*>(MAKELONG(g_userdata_atom, 0))));
#elif defined(METHOD3)
    auto it = g_userdata.find(hwnd);
    if (it != g_userdata.end())
        return it->second;
    else
        return nullptr;
#elif defined(METHOD4)
    auto it = g_userdata.find(hwnd);
    if (it != g_userdata.end())
        return it->second;
    else
        return nullptr;
#elif defined(METHOD5)
    userdata_item item{hwnd, nullptr};
    auto it = std::lower_bound(g_userdata.begin(), g_userdata.end(), item);
    if (it != g_userdata.end() && it->hwnd == hwnd)
        return it->data;
    else
        return nullptr;
#endif
}