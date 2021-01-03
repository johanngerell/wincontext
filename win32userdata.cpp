#include "win32userdata.h"
#include <unordered_map> // impl_4
#include <map> // impl_5
#include <vector> // impl_0, impl_6
#include <algorithm> // impl_6

struct userdata_0 final : public userdata
{
    std::vector<void*> data;
    int size;
    int index;

    userdata_0(int size_hint)
        : size{size_hint}
        , data{size_hint}
        , index{0}
    {}

    virtual const char* description() override
    {
        return "Baseline indexed array userdata";
    }

    virtual void set_impl(HWND, void* data_) override
    {
        data[index] = data_;
        if (++index >= size)
            index = 0;
    }

    virtual void* get_impl(HWND) override
    {
        void* d = data[index];
        if (++index >= size)
            index = 0;
        return d;
    }
};

struct userdata_1 final : public userdata
{
    virtual const char* description() override
    {
        return "Win32 window userdata";
    }

    virtual void set_impl(HWND hwnd, void* data) override
    {
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(data));
    }

    virtual void* get_impl(HWND hwnd) override
    {
        return reinterpret_cast<void*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
    }
};

struct userdata_2 final : public userdata
{
    virtual const char* description() override
    {
        return "Win32 window property, string id";
    }

    virtual void set_impl(HWND hwnd, void* data) override
    {
        SetPropA(hwnd, "userdata", data);
    }

    virtual void* get_impl(HWND hwnd) override
    {
        return reinterpret_cast<void*>(GetPropA(hwnd, "userdata"));
    }
};

struct userdata_3 final : public userdata
{
    ATOM userdata_atom = GlobalAddAtomA("userdata");

    virtual const char* description() override
    {
        return "Win32 window property, atom id";
    }

    virtual void set_impl(HWND hwnd, void* data) override
    {
        SetPropA(hwnd, reinterpret_cast<const char*>(static_cast<long long>(MAKELONG(userdata_atom, 0))), data);
    }

    virtual void* get_impl(HWND hwnd) override
    {
        return reinterpret_cast<void*>(GetPropA(hwnd, reinterpret_cast<const char*>(static_cast<long long>(MAKELONG(userdata_atom, 0)))));
    }
};

struct userdata_4 final : public userdata
{
    std::unordered_map<HWND, void*> data;

    virtual const char* description() override
    {
        return "std::unordered_map";
    }

    virtual void set_impl(HWND hwnd, void* data_) override
    {
        data[hwnd] = data_;
    }

    virtual void* get_impl(HWND hwnd) override
    {
        auto it = data.find(hwnd);
        if (it != data.end())
            return it->second;
        else
            return nullptr;
    }
};

struct userdata_5 final : public userdata
{
    std::map<HWND, void*> data;

    virtual const char* description() override
    {
        return "std::map";
    }

    virtual void set_impl(HWND hwnd, void* data_) override
    {
        data[hwnd] = data_;
    }

    virtual void* get_impl(HWND hwnd) override
    {
        auto it = data.find(hwnd);
        if (it != data.end())
            return it->second;
        else
            return nullptr;
    }
};

struct userdata_6 final : public userdata
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

    std::vector<userdata_item> data;

    virtual const char* description() override
    {
        return "std::vector, sorted";
    }

    virtual void set_impl(HWND hwnd, void* data_) override
    {
        userdata_item item{hwnd, data_};
        data.insert(std::upper_bound(data.begin(), data.end(), item), item);
    }

    virtual void* get_impl(HWND hwnd) override
    {
        userdata_item item{hwnd, nullptr};
        auto it = std::lower_bound(data.begin(), data.end(), item);
        if (it != data.end() && it->hwnd == hwnd)
            return it->data;
        else
            return nullptr;
    }
};

struct userdata_7 final : public userdata
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

    std::vector<userdata_item> data;

    virtual const char* description() override
    {
        return "std::vector, unsorted";
    }

    virtual void set_impl(HWND hwnd, void* data_) override
    {
        userdata_item item{hwnd, data_};
        auto it = std::find(data.begin(), data.end(), item);
        if (it != data.end())
            it->data = data_;
        else
            data.push_back(item);
    }

    virtual void* get_impl(HWND hwnd) override
    {
        userdata_item item{hwnd, nullptr};
        auto it = std::find(data.begin(), data.end(), item);
        if (it != data.end())
            return it->data;
        else
            return nullptr;
    }
};

std::unique_ptr<userdata> make_userdata(int impl_index, int size_hint)
{
    switch (impl_index)
    {
        case 0: return std::make_unique<userdata_0>(size_hint);
        case 1: return std::make_unique<userdata_1>();
        case 2: return std::make_unique<userdata_2>();
        case 3: return std::make_unique<userdata_3>();
        case 4: return std::make_unique<userdata_4>();
        case 5: return std::make_unique<userdata_5>();
        case 6: return std::make_unique<userdata_6>();
        case 7: return std::make_unique<userdata_7>();    
        default: return nullptr;
    }
}
