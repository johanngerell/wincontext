@echo off

: cl.exe /nologo /EHsc /Od /Zi main.cpp /link /SUBSYSTEM:WINDOWS /out:wincontext.exe user32.lib

cl.exe /nologo /Od /Zi main.cpp /link /SUBSYSTEM:WINDOWS /out:wincontext.exe user32.lib
