@echo off

if "%1"=="debug" (
    set "flags=/Od"
) else if "%1"=="release" (
    set "flags=/O2"
) else (
    echo Syntax: %0 [debug^|release]
    exit /b 1
)

set "output=wincontext.exe"
if exist "%output%" del %output%

cl.exe /nologo %flags% /Zi main.cpp userdata.cpp win32api.cpp /link /SUBSYSTEM:WINDOWS /out:%output% user32.lib

