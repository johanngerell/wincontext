#include <windows.h>
#include <stdexcept>

class win32_error : std::runtime_error
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

HWND create_window(HWND parent, const char* class_name, const char* text, DWORD style,
                   int x, int y, int width, int height, void* context = nullptr);

SIZE window_size_for_client(int width, int height, DWORD style);
