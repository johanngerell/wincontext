#pragma once

#include <stdexcept>
#include "win32_api.h"

class win32_error : public std::runtime_error
{
public:
    win32_error(const char* function, DWORD code)
        : std::runtime_error{function}
        , m_code{code}
    {}

    DWORD code() const { return m_code; }

private:
    DWORD m_code{};
};
