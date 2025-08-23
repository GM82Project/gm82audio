@echo off

cmake -A Win32 -B build && cmake --build build --config Release
py gm82gex.py gm82audio.gej

pause