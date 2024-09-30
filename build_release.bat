@echo off

del gm82cs.dll

cmake -B build && cmake --build build --config Release

move build\Release\gm82cs.dll gm82cs.dll

pause