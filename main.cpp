#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "win32userdata.h"
#include "win32api.h"

constexpr int row_count{10};
constexpr int column_count{10};
constexpr int layer_count{10};
constexpr int label_count{row_count * column_count * layer_count};

struct grid_location
{
    int row{};
    int column{};
    int layer{};
};

grid_location grid_location_from_index(int index)
{
    return
    {
        (index % (row_count * column_count)) / column_count,
        (index % (row_count * column_count)) % column_count,
         index / (row_count * column_count)
    };
}

constexpr int cell_spacing{10};
constexpr SIZE cell_size{20, 20};

SIZE client_size_for_grid()
{
    return
    {
        column_count * (cell_size.cx + cell_spacing) + cell_spacing + layer_count * 2,
        row_count * (cell_size.cy + cell_spacing) + cell_spacing + layer_count * 2
    };
};

POINT position_from_grid_location(const grid_location& location)
{
    return
    {
        cell_spacing + cell_size.cy * location.row + cell_spacing * location.row + location.layer * 2,
        cell_spacing + cell_size.cx * location.column + cell_spacing * location.column + location.layer * 2
    };
}

template <typename Func>
std::chrono::microseconds::rep benchmark_average_us(int sample_count, Func&& func)
{
    auto t1 = std::chrono::high_resolution_clock::now();

    for (int sample = 0; sample < sample_count; ++sample)
        func();

    auto t2 = std::chrono::high_resolution_clock::now();                
    
    return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / sample_count;
}

using message_map = std::unordered_map<UINT, std::function<LRESULT(HWND, WPARAM, LPARAM)>>;

class window
{
public:
    window() = default;

    window(HWND parent, POINT position, SIZE size, DWORD style, const char* text, const char* class_name)
    {
        window_info label_create_info;
        label_create_info.parent     = parent;
        label_create_info.class_name = class_name;
        label_create_info.text       = text;
        label_create_info.style      = style;
        label_create_info.size       = size;
        label_create_info.position   = position;

        m_hwnd = create_window(label_create_info);
    }

    HWND hwnd() const
    {
        return m_hwnd;
    };

protected:
    HWND m_hwnd;
};

std::string benchmark_userdata_access(const std::vector<window>& labels)
{
    const auto average_us = benchmark_average_us(100, [&labels]
    {
        for (const auto& label : labels)
            get_userdata<int>(label.hwnd()) += 1;
    });

    std::string text("average time: ");
    text += std::to_string(average_us);
    text += " us (";
    text += get_userdata_description();
    text += ")";

    return text;
}

void initialize_labels(HWND parent, std::vector<window>& labels, std::vector<int>& values)
{
    // Since we need the positions to be stable as userdata
    labels.reserve(label_count);
    values.reserve(label_count);

    // Layout all labels in a grid
    for (int i = 0; i < label_count; ++i)
    {
        const auto location = grid_location_from_index(i);
        const auto position = position_from_grid_location(location);
        labels.push_back(window(parent, position, cell_size, SS_BLACKFRAME, "", "STATIC"));
        values.push_back(rand() % 4711);
        set_userdata(labels.back().hwnd(), &values.back());
    }
}

class message_window : public window
{
public:
    message_window(POINT position, SIZE client_size, DWORD style, const char* title, const char* class_name)
    {
        window_info info;
        info.class_name = register_window_class(wndproc, class_name);
        info.text       = title;
        info.style      = style;
        info.position   = position;
        info.size       = window_size_for_client(client_size, style);

        m_hwnd = create_window(info);
        set_userdata(m_hwnd, this);
    }

    void on(UINT msg, std::function<LRESULT(HWND, WPARAM, LPARAM)> func)
    {
        m_message_map[msg] = std::move(func);
    }

    void on(UINT msg, LRESULT result, std::function<void(HWND, WPARAM, LPARAM)> func)
    {
        m_message_map[msg] = [func = std::move(func), result] (HWND hwnd_, WPARAM wp, LPARAM lp)
        {
            func(hwnd_, wp, lp);
            return result;
        };
    }

private:
    static LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        if (auto self = try_get_userdata<message_window>(hwnd))
            if (auto it = self->m_message_map.find(msg); it != self->m_message_map.end())
                return it->second(hwnd, wp, lp);

        return DefWindowProc(hwnd, msg, wp, lp);
    }

    message_map m_message_map{};
};

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    message_window popup({100, 100}, client_size_for_grid(), WS_POPUPWINDOW | WS_CAPTION,
                         "Press 'g' to measure userdata access", "Window Context Test Class");

    // Hold an int value as userdata for each label
    std::vector<window> labels;
    std::vector<int> label_values;
    initialize_labels(popup.hwnd(), labels, label_values);

    popup.on(WM_CHAR, 0, [&labels] (HWND hwnd, WPARAM wp, LPARAM) 
    {
        switch (static_cast<char>(wp))
        {
            case 'g': SetWindowTextA(hwnd, benchmark_userdata_access(labels).c_str()); break;
            case 'q': DestroyWindow(hwnd); break;
        }
    });

    popup.on(WM_DESTROY, 0, [] (HWND, WPARAM, LPARAM)
    {
        PostQuitMessage(0);
    });

    simple_message_loop();

    return 1;
}