@echo off
setlocal
rd /s /q build
if not exist build (
    mkdir build
)

cd build

echo [BUILD] Configuring project...
cmake ..
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed.
    pause
    exit /b %errorlevel%
)

echo [BUILD] Building...
cmake --build .
if %errorlevel% neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b %errorlevel%
)

echo [SUCCESS] Running app...
stackio.exe

cd ..
endlocal