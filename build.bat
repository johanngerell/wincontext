@echo off

if "%1"=="debug" (
    set "flags=/Od"
) else if "%1"=="release" (
    set "flags=/O2"
) else (
    echo Syntax: %0 [debug^|release]
    exit /b 1
)

set "output_dir=%1"
if not exist "%output_dir%" mkdir %output_dir%

set "output_exe=%output_dir%\wincontext.exe"

cl.exe /nologo %flags% /W4 /WX /Zi /Fe"%output_exe%" /Fo"%output_dir%/" /Fd"%output_dir%/" main.cpp userdata.cpp win32api.cpp /link /SUBSYSTEM:WINDOWS user32.lib
