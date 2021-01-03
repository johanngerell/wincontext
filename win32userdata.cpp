#include "win32userdata.h"
#include <array> // impl_0
#include <unordered_map> // impl_4
#include <map> // impl_5
#include <vector> // impl_6
#include <algorithm> // impl_6

struct impl_0
{
    std::array<void*, 1000> userdata;
} g_impl_0;

struct impl_1
{} g_impl_1;

struct impl_2
{} g_impl_2;

struct impl_3
{
    ATOM userdata_atom = GlobalAddAtomA("userdata");
}  g_impl_3;

struct impl_4
{
    std::unordered_map<HWND, void*> userdata;
} g_impl_4;

struct impl_5
{
    std::map<HWND, void*> userdata;
} g_impl_5;

struct impl_6
{
    struct userdata_item
    {
        HWND hwnd;
        void* data;

        bool operator<(const userdata_item& other) const
        {
            return hwnd < other.hwnd;
        }
    };

    std::vector<userdata_item> userdata;
} g_impl_6;

struct impl_7
{
    struct userdata_item
    {
        HWND hwnd;
        void* data;

        bool operator==(const userdata_item& other) const
        {
            return hwnd == other.hwnd;
        }
    };

    std::vector<userdata_item> userdata;
} g_impl_7;

const char* get_userdata_description(int impl_index)
{
    switch (impl_index)
    {
        case 0: return "Baseline indexed array userdata";
        case 1: return "Win32 window userdata";
        case 2: return "Win32 window property, string id";
        case 3: return "Win32 window property, atom id";
        case 4: return "std::unordered_map";
        case 5: return "std::map";
        case 6: return "std::vector, sorted";
        case 7: return "std::vector, unsorted";
        default: return "UNKNOWN";
    }
}

void set_userdata_impl(int impl_index, int hwnd_index, HWND hwnd, void* userdata)
{
    switch (impl_index)
    {
        case 0: g_impl_0.userdata[hwnd_index] = userdata; break;
        case 1: SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(userdata)); break;
        case 2: SetPropA(hwnd, "userdata", userdata); break;
        case 3: SetPropA(hwnd, reinterpret_cast<const char*>(static_cast<long long>(MAKELONG(g_impl_3.userdata_atom, 0))), userdata); break;
        case 4: g_impl_4.userdata[hwnd] = userdata; break;
        case 5: g_impl_5.userdata[hwnd] = userdata; break;
        case 6:
        {
            impl_6::userdata_item item{hwnd, userdata};
            g_impl_6.userdata.insert(std::upper_bound(g_impl_6.userdata.begin(), g_impl_6.userdata.end(), item), item);
            break;
        }
        case 7:
        {
            impl_7::userdata_item item{hwnd, userdata};
            auto it = std::find(g_impl_7.userdata.begin(), g_impl_7.userdata.end(), item);
            if (it != g_impl_7.userdata.end())
                it->data = userdata;
            else
                g_impl_7.userdata.push_back(item);
             break;
        }
    }
}

void* get_userdata_impl(int impl_index, int hwnd_index, HWND hwnd)
{
    switch (impl_index)
    {
        case 0: return g_impl_0.userdata[hwnd_index];
        case 1: return reinterpret_cast<void*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
        case 2: return reinterpret_cast<void*>(GetPropA(hwnd, "userdata"));
        case 3: return reinterpret_cast<void*>(GetPropA(hwnd, reinterpret_cast<const char*>(static_cast<long long>(MAKELONG(g_impl_3.userdata_atom, 0)))));
        case 4:
        {
            auto it = g_impl_4.userdata.find(hwnd);
            if (it != g_impl_4.userdata.end())
                return it->second;
            else
                return nullptr;
        }
        case 5:
        {
            auto it = g_impl_5.userdata.find(hwnd);
            if (it != g_impl_5.userdata.end())
                return it->second;
            else
                return nullptr;
        }
        case 6:
        {
            impl_6::userdata_item item{hwnd, nullptr};
            auto it = std::lower_bound(g_impl_6.userdata.begin(), g_impl_6.userdata.end(), item);
            if (it != g_impl_6.userdata.end() && it->hwnd == hwnd)
                return it->data;
            else
                return nullptr;
        }
        case 7:
        {
            impl_7::userdata_item item{hwnd, nullptr};
            auto it = std::find(g_impl_7.userdata.begin(), g_impl_7.userdata.end(), item);
            if (it != g_impl_7.userdata.end())
                return it->data;
            else
                return nullptr;
        }
        default: return nullptr;
    }
}
