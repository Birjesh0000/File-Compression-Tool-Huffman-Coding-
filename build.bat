@echo off
REM Add MSYS2 to PATH and compile

echo.
echo Huffman File Compressor/Decompressor - Build Script (MSYS2)
echo =========================================================
echo.

REM Add MSYS2 UCRT64 to PATH temporarily
set PATH=C:\msys64\ucrt64\bin;%PATH%

echo Compiling compress.cpp...
g++ -std=c++17 -static-libgcc -static-libstdc++ -o compress.exe compress.cpp
if %errorlevel% neq 0 (
    echo Error compiling compress.cpp
    exit /b 1
)

echo Compiling decompress.cpp...
g++ -std=c++17 -static-libgcc -static-libstdc++ -o decompress.exe decompress.cpp
if %errorlevel% neq 0 (
    echo Error compiling decompress.cpp
    exit /b 1
)

echo.
echo ✓ Build completed successfully!
echo ✓ Executables created: compress.exe, decompress.exe
echo.
echo Usage:
echo   compress.exe input_file output.huff
echo   decompress.exe output.huff recovered_file
echo.
