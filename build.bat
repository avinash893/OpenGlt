@echo off
echo Building OpenGlt project with CMake...

REM Create build directory if it doesn't exist
if not exist "build" mkdir build
cd build

REM Configure the project
echo Configuring project...
cmake .. -G "Visual Studio 17 2022" -A x64

REM Build the project
echo Building project...
cmake --build . --config Debug

echo Build completed!
echo Executable location: build\bin\Debug\OpenGlt.exe
pause
