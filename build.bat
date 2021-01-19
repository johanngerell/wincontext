@echo off

setlocal

if "%1"=="test" set "optimization=/RTCscu /D_ALLOW_RTCc_IN_STL /Od" && goto :build
if "%1"=="debug" set "optimization=/RTCscu /D_ALLOW_RTCc_IN_STL /Od" && goto :build
if "%1"=="release" set "optimization=/O2" && goto :build

echo Syntax: %0 [debug^|release^|test] && exit /b 1

:build

if "%1"=="test" (
    set "input=test.cpp"
    set "output_dir=test"
    set "output_exe=test.exe"
    set "lflags=/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF"
) else (
    set "input=main.cpp win32_userdata.cpp win32_api.cpp"
    set "output_dir=%1"
    set "output_exe=wincontext.exe"
    set "lflags=/SUBSYSTEM:WINDOWS /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF user32.lib"
)

set "output=/Fe^"%output_dir%\%output_exe%^" /Fo^"%output_dir%/^" /Fd^"%output_dir%/^""
set "cflags=/nologo /EHsc %optimization% /W4 /WX /Zi /std:c++17 %output% %input%"

if not exist "%output_dir%" mkdir %output_dir%

cl.exe %cflags% /link %lflags%
