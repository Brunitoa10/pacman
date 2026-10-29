@echo off
setlocal
REM Compila la version grafica (raylib) -> pacman_gui.exe
REM
REM RAYLIB_DIR puede apuntar a:
REM   - la instalacion de raylib para Windows (por defecto C:\raylib\raylib\src)
REM   - o una release descomprimida "raylib-5.5_win64_mingw-w64" (con include\ y lib\)
REM Ejemplo: set RAYLIB_DIR=C:\libs\raylib-5.5_win64_mingw-w64 & build_raylib.bat

if "%RAYLIB_DIR%"=="" set RAYLIB_DIR=C:\raylib\raylib\src

if exist "%RAYLIB_DIR%\include\raylib.h" (
    set RL_INC=%RAYLIB_DIR%\include
    set RL_LIB=%RAYLIB_DIR%\lib
) else (
    set RL_INC=%RAYLIB_DIR%
    set RL_LIB=%RAYLIB_DIR%
)

if not exist "%RL_INC%\raylib.h" (
    echo No encuentro raylib.h en "%RL_INC%".
    echo Instala raylib ^(https://www.raylib.com^) o defini RAYLIB_DIR.
    pause
    exit /b 1
)

REM Si existe el gcc que trae el instalador de raylib, se usa ese.
if exist "C:\raylib\w64devkit\bin\gcc.exe" set PATH=C:\raylib\w64devkit\bin;%PATH%
where gcc >nul 2>nul
if errorlevel 1 (
    echo [ERROR] No se encontro gcc. Instala MinGW-w64 o raylib ^(trae gcc en C:\raylib\w64devkit\bin^).
    pause
    exit /b 1
)

gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -Iinclude -I"%RL_INC%" ^
    src\main.c src\core\*.c src\view\*.c src\platform\platform_clock.c src\platform\raylib\*.c ^
    -L"%RL_LIB%" -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows -o pacman_gui.exe
if errorlevel 1 (
    echo [ERROR] Fallo la compilacion.
    pause
    exit /b 1
)
echo Listo: pacman_gui.exe
