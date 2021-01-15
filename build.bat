@echo off

if "%1"=="debug" set "optimization=/RTCscu /D_ALLOW_RTCc_IN_STL /Od" && goto :build
if "%1"=="release" set "optimization=/O2" && goto :build

echo Syntax: %0 [debug^|release] && exit /b 1

:build

set "input=main.cpp win32_userdata.cpp win32_api.cpp"
set "output_dir=%1"
set "output_exe=%output_dir%\wincontext.exe"
set "output=/Fe^"%output_exe%^" /Fo^"%output_dir%/^" /Fd^"%output_dir%/^""
set "cflags=/nologo /EHsc %optimization% /W4 /WX /Zi /std:c++17 %output% %input%"
set "libs=user32.lib"
set "lflags=/SUBSYSTEM:WINDOWS /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF %libs%"

if not exist "%output_dir%" mkdir %output_dir%

: echo cl.exe %cflags% /link %lflags%

cl.exe %cflags% /link %lflags%
