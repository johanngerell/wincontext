cmake_minimum_required(VERSION 3.10)

project(wincontext)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTCscu /D_ALLOW_RTCc_IN_STL")

include_directories(submodules/jg/inc)

string(REGEX REPLACE "/W[1-3]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
add_compile_options(/Zc:__cplusplus /EHsc /W4 /WX /Zi)
add_link_options(/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF)

add_executable(wincontext WIN32 src/main.cpp src/win32_userdata.cpp src/win32_api.cpp)
add_executable(tests src/test.cpp)
