@echo off

del gm82cs.dll

cmake -A Win32 -B build && cmake --build build --config Release

move build\Release\gm82cs.dll gm82cs.dll

pause