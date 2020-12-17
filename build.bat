@echo off

if "%1"=="debug" set "optimization=/Od" && goto :build
if "%1"=="release" set "optimization=/O2" && goto :build

echo Syntax: %0 [debug^|release] && exit /b 1

:build

set "output_dir=%1"
set "output_exe=%output_dir%\wincontext.exe"
set "output=/Fe^"%output_exe%^" /Fo^"%output_dir%/^" /Fd^"%output_dir%/^""
set "input=main.cpp win32userdata.cpp win32api.cpp"
set "flags=/nologo /EHsc %optimization% /W4 /WX /Zi /std:c++17"

if not exist "%output_dir%" mkdir %output_dir%

cl.exe %flags% %output% %input% /link /SUBSYSTEM:WINDOWS user32.lib
