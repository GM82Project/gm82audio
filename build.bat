@echo off

cmake -A Win32 -B build && cmake --build build --config Release

pause
