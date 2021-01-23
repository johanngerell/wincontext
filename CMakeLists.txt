cmake_minimum_required(VERSION 3.10)

project(wincontext)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTCscu /D_ALLOW_RTCc_IN_STL")
    string(REGEX REPLACE "/W[1-3]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

include_directories(submodules/jg/inc)

if (MSVC)
    add_compile_options(/Zc:__cplusplus /EHsc /W4 /WX /Zi)
    add_link_options(/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF)
# else()
#     # jg::mock can't be used with -pedantic due to "the ##__VA_ARGS__ trick"
#     add_compile_options(-Wall -Wextra -Werror)
endif()

if(MSVC)
    add_executable(wincontext WIN32 src/main.cpp src/win32_userdata.cpp src/win32_api.cpp)
else()
    add_executable(wincontext src/main.cpp src/win32_userdata.cpp src/win32_api.cpp)
endif()

add_executable(tests src/test.cpp)
