#include <windows.h>
#include <stdexcept>

class win32_error : public std::runtime_error
{
public:
    explicit win32_error(DWORD code, const char* function)
        : std::runtime_error(function)
        , m_code{code}
    {}

    DWORD code() const { return m_code; }

private:
    DWORD m_code{};
};

const char* register_window_class(WNDPROC wndproc, const char* name);

struct window_info
{
    const char* class_name{};
    const char* text{};
    DWORD style{};
    POINT position{};
    SIZE size{};
    HWND parent{};
};

HWND create_window(const window_info& info);

SIZE window_size_for_client(SIZE client_size, DWORD window_style);
