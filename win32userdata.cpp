#include "win32userdata.h"
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>
#include <memory>

namespace
{

struct userdata
{
    virtual const char* description() = 0;

    virtual void set(HWND hwnd, void* data) = 0;
    virtual void* get(HWND hwnd) = 0;

    virtual ~userdata() = default;
};

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
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(data));
    }

    virtual void* get(HWND hwnd) override
    {
        return reinterpret_cast<void*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
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
        SetPropA(hwnd, "userdata", data);
    }

    virtual void* get(HWND hwnd) override
    {
        return reinterpret_cast<void*>(GetPropA(hwnd, "userdata"));
    }
};

struct userdata_3 final : userdata
{
    ATOM userdata_atom = GlobalAddAtomA("userdata");

    virtual const char* description() override
    {
        return "Win32 window property, atom id";
    }

    virtual void set(HWND hwnd, void* data) override
    {
        SetPropA(hwnd, reinterpret_cast<const char*>(static_cast<long long>(MAKELONG(userdata_atom, 0))), data);
    }

    virtual void* get(HWND hwnd) override
    {
        return reinterpret_cast<void*>(GetPropA(hwnd, reinterpret_cast<const char*>(static_cast<long long>(MAKELONG(userdata_atom, 0)))));
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
    struct userdata_item
    {
        HWND hwnd;
        void* data;
    };

    std::vector<userdata_item> data;

    virtual const char* description() override
    {
        return "std::vector, sorted";
    }

    virtual void set(HWND hwnd, void* data_) override
    {
        userdata_item item{hwnd, data_};
        data.insert(std::upper_bound(data.begin(), data.end(), item, [](auto& first, auto& second) { return first.hwnd < second.hwnd; }), item);
    }

    virtual void* get(HWND hwnd) override
    {
        userdata_item item{hwnd, nullptr};
        auto it = std::lower_bound(data.begin(), data.end(), item, [](auto& first, auto& second) { return first.hwnd < second.hwnd; });
        return it != data.end() && it->hwnd == hwnd ? it->data : nullptr;
    }
};

struct userdata_7 final : userdata
{
    struct userdata_item
    {
        HWND hwnd;
        void* data;
    };

    std::vector<userdata_item> data;

    virtual const char* description() override
    {
        return "std::vector, unsorted";
    }

    virtual void set(HWND hwnd, void* data_) override
    {
        userdata_item item{hwnd, data_};        
        if (auto it = std::find_if(data.begin(), data.end(), [&item](auto& other){ return item.hwnd == other.hwnd; }); it != data.end())
            it->data = data_;
        else
            data.push_back(item);
    }

    virtual void* get(HWND hwnd) override
    {
        userdata_item item{hwnd, nullptr};
        auto it = std::find_if(data.begin(), data.end(), [&item](auto& other){ return item.hwnd == other.hwnd; });
        return it != data.end() ? it->data : nullptr;
    }
};

std::unique_ptr<userdata> make_userdata(userdata_kind kind)
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

std::unique_ptr<userdata> g_userdata;

}

void userdata_init(userdata_kind kind)
{
    g_userdata = make_userdata(kind);
}

const char* userdata_description()
{
    return g_userdata ? g_userdata->description() : nullptr;
}

void userdata_set(HWND hwnd, void* data)
{
    if (g_userdata)
        g_userdata->set(hwnd, data);
}

void* userdata_get(HWND hwnd)
{
    return g_userdata ? g_userdata->get(hwnd) : nullptr;
}
