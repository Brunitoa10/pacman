@echo off
REM Compila Pac-Man con MinGW (gcc en el PATH).
if exist "C:\raylib\w64devkit\bin\gcc.exe" set PATH=C:\raylib\w64devkit\bin;%PATH%
where gcc >nul 2>nul
if errorlevel 1 (
    echo [ERROR] No se encontro gcc. Instala MinGW-w64 o raylib ^(trae gcc en C:\raylib\w64devkit\bin^).
    pause
    exit /b 1
)
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -Iinclude src\main.c src\core\*.c src\view\*.c src\platform\*.c -o pacman.exe
if errorlevel 1 (
    echo [ERROR] Fallo la compilacion.
    pause
    exit /b 1
)
echo Listo: pacman.exe

if /I "%1"=="test" (
    gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -Iinclude tests\test_main.c src\core\*.c src\view\*.c -o pacman_tests.exe && pacman_tests.exe
)
