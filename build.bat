@echo off
setlocal

:: Move to the directory where the .bat file is located
cd /d "%~dp0"

:: Clean and recreate build directory
if exist build (
    rd /s /q build
)
mkdir build

cd build

echo [BUILD] Configuring project...
cmake ..
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed.
    pause
    exit /b %errorlevel%
)

echo [BUILD] Building...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b %errorlevel%
)

echo [SUCCESS] Running app...
:: Go back to root so the relative path "./config.xml" works correctly
cd ..

:: Check if the exe is in build/ or build/Release/ (CMake behavior on Windows)
if exist "build\Release\stackio.exe" (
    "build\Release\stackio.exe" "config.xml"
) else (
    "build\stackio.exe" "config.xml"
)

if %errorlevel% neq 0 (
    echo [ERROR] Application exited with code %errorlevel%
)

pause
endlocal