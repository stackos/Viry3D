@echo off
if not exist build\win_x64 (
    mkdir build\win_x64
)
cd build\win_x64
cmake ..\..\ -G "Visual Studio 16 2019" -A x64 -DTarget=Windows -DArch=x64
cd ..\..\
pause