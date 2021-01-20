@echo off

setlocal

if "%1"=="test" set "optimization=/RTCscu /D_ALLOW_RTCc_IN_STL /Od" && goto :build
if "%1"=="debug" set "optimization=/RTCscu /D_ALLOW_RTCc_IN_STL /Od" && goto :build
if "%1"=="release" set "optimization=/O2" && goto :build

echo Syntax: %0 [debug^|release^|test] && exit /b 1

:build

if "%1"=="test" (
    set "input=src\test.cpp"
    set "output_dir=test"
    set "output_exe=test.exe"
    set "lflags=/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF"
) else (
    set "input=src\main.cpp src\win32_userdata.cpp src\win32_api.cpp"
    set "output_dir=%1"
    set "output_exe=wincontext.exe"
    set "lflags=/SUBSYSTEM:WINDOWS /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF user32.lib"
)

set "inc_dir=/I submodules\jg\inc"
set "output=/Fe^"build\%output_dir%\%output_exe%^" /Fo^"build\%output_dir%/^" /Fd^"build\%output_dir%/^""
set "cflags=/nologo /EHsc %optimization% /W4 /WX /Zi /std:c++17 %inc_dir% %output% %input%"

if not exist "build\%output_dir%" mkdir build\%output_dir%

cl.exe %cflags% /link %lflags%
