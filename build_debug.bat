@echo off

del gm82audio.dll

cmake -A Win32 -B build && cmake --build build --config Debug

move build\Debug\gm82audio.dll gm82audio.dll

py gm82gex.py gm82audio.gej

pause