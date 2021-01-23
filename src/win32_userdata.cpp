#include "win32_userdata.h"
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>
#include <functional>

namespace
{

struct userdata_0 final : userdata
{
    std::vector<void*> data;
    int index{};
    bool doing_set{};
    bool doing_get{};

    virtual const char* description() override
    {
        return "Baseline indexed array userdata";
    }

    virtual void set(HWND, void* data_) override
    {
        data.push_back(data_);
        doing_set = true;
        doing_get = false;
    }

    virtual void* get(HWND) override
    {
        if (doing_get)
        {
            if (++index >= data.size())
                index = 0;
        }
        else if (doing_set)
        {
            index = 0;
            doing_set = false;
            doing_get = true;
        }
        else
            return nullptr;

        return data[index];
    }
};

struct userdata_1 final : userdata
{
    virtual const char* description() override
    {
        return "Win32 window userdata";
    }

    virtual void set(HWND hwnd, void* data) override
    {
#ifdef _WIN32
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(data));
#else
        (void)hwnd; (void)data;
#endif
    }

    virtual void* get(HWND hwnd) override
    {
#ifdef _WIN32
        return reinterpret_cast<void*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
#else
        (void)hwnd;
        return nullptr;
#endif
    }
};

struct userdata_2 final : userdata
{
    virtual const char* description() override
    {
        return "Win32 window property, string id";
    }

    virtual void set(HWND hwnd, void* data) override
    {
#ifdef _WIN32
        SetPropA(hwnd, "userdata", data);
#else
        (void)hwnd; (void)data;
#endif
    }

    virtual void* get(HWND hwnd) override
    {
#ifdef _WIN32
        return reinterpret_cast<void*>(GetPropA(hwnd, "userdata"));
#else
        (void)hwnd;
        return nullptr;
#endif
    }
};

struct userdata_3 final : userdata
{
#ifdef _WIN32
    ATOM userdata_atom = GlobalAddAtomA("userdata");
#endif

    virtual const char* description() override
    {
        return "Win32 window property, atom id";
    }

    virtual void set(HWND hwnd, void* data) override
    {
#ifdef _WIN32
        SetPropA(hwnd, reinterpret_cast<const char*>(static_cast<long long>(MAKELONG(userdata_atom, 0))), data);
#else
        (void)hwnd; (void)data;
#endif
    }

    virtual void* get(HWND hwnd) override
    {
#ifdef _WIN32
        return reinterpret_cast<void*>(GetPropA(hwnd, reinterpret_cast<const char*>(static_cast<long long>(MAKELONG(userdata_atom, 0)))));
#else
        (void)hwnd;
        return nullptr;
#endif
    }
};

struct userdata_4 final : userdata
{
    std::unordered_map<HWND, void*> data;

    virtual const char* description() override
    {
        return "std::unordered_map";
    }

    virtual void set(HWND hwnd, void* data_) override
    {
        data[hwnd] = data_;
    }

    virtual void* get(HWND hwnd) override
    {
        auto it = data.find(hwnd);
        return it != data.end() ? it->second : nullptr;
    }
};

struct userdata_5 final : userdata
{
    std::map<HWND, void*> data;

    virtual const char* description() override
    {
        return "std::map";
    }

    virtual void set(HWND hwnd, void* data_) override
    {
        data[hwnd] = data_;
    }

    virtual void* get(HWND hwnd) override
    {
        auto it = data.find(hwnd);
        return it != data.end() ? it->second : nullptr;
    }
};

struct userdata_6 final : userdata
{
    using data_item = std::pair<HWND, void*>;
    std::vector<std::pair<HWND, void*>> data;

    virtual const char* description() override
    {
        return "std::vector, sorted";
    }

    virtual void set(HWND hwnd, void* data_) override
    {
        data_item item{hwnd, data_};
        auto it = std::upper_bound(data.begin(), data.end(), item, less_hwnd);
        data.insert(it, std::move(item));
    }

    virtual void* get(HWND hwnd) override
    {
        data_item item{hwnd, nullptr};
        if (auto it = std::lower_bound(data.begin(), data.end(), item, less_hwnd); it != data.end() && it->first == hwnd)
            return it->second;
        else
            return nullptr;
    }

    static bool less_hwnd(const data_item& item1, const data_item& item2)
    {
        return item1.first < item2.first;
    }
};

struct userdata_7 final : userdata
{
    using data_item = std::pair<HWND, void*>;
    std::vector<data_item> data;

    virtual const char* description() override
    {
        return "std::vector, unsorted";
    }

    virtual void set(HWND hwnd, void* data_) override
    {
        data_item item{hwnd, data_};
        if (auto it = std::find_if(data.begin(), data.end(), std::bind(equal_hwnd, item, std::placeholders::_1)); it != data.end())
            it->second = data_;
        else
            data.push_back(std::move(item));
    }

    virtual void* get(HWND hwnd) override
    {
        data_item item{hwnd, nullptr};
        if (auto it = std::find_if(data.begin(), data.end(), std::bind(equal_hwnd, item, std::placeholders::_1)); it != data.end())
            return it->second;
        else
            return nullptr;
    }

    static bool equal_hwnd(const data_item& item1, const data_item& item2)
    {
        return item1.first == item2.first;
    }
};

}

std::unique_ptr<userdata> create_userdata(userdata_kind kind)
{
    switch (kind)
    {
        case userdata_kind::baseline:            return std::make_unique<userdata_0>();
        case userdata_kind::get_set_window_long: return std::make_unique<userdata_1>();
        case userdata_kind::get_set_prop:        return std::make_unique<userdata_2>();
        case userdata_kind::get_set_prop_atom:   return std::make_unique<userdata_3>();
        case userdata_kind::unordered_map:       return std::make_unique<userdata_4>();
        case userdata_kind::map:                 return std::make_unique<userdata_5>();
        case userdata_kind::vector_sorted:       return std::make_unique<userdata_6>();
        case userdata_kind::vector_unsorted:     return std::make_unique<userdata_7>();    
        default: return nullptr;
    }
}
