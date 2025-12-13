@echo off
REM Huffman Compressor/Decompressor Build Script
REM This script attempts to compile the project with available compilers

echo.
echo Huffman File Compressor/Decompressor - Build Script
echo ====================================================
echo.

REM Try to find and use available C++ compiler
where /q clang++
if %errorlevel% equ 0 (
    echo Found clang++. Compiling...
    clang++ -std=c++17 -o compress.exe compress.cpp
    clang++ -std=c++17 -o decompress.exe decompress.cpp
    goto :success
)

where /q g++
if %errorlevel% equ 0 (
    echo Found g++. Compiling...
    g++ -std=c++17 -o compress.exe compress.cpp
    g++ -std=c++17 -o decompress.exe decompress.cpp
    goto :success
)

where /q cl
if %errorlevel% equ 0 (
    echo Found MSVC cl.exe. Compiling...
    cl /std:latest /EHsc compress.cpp /Fe:compress.exe
    cl /std:latest /EHsc decompress.cpp /Fe:decompress.exe
    goto :success
)

echo Error: No C++ compiler found!
echo Please install one of: MSVC, GCC, or Clang
exit /b 1

:success
echo.
echo Build completed!
echo Executables: compress.exe, decompress.exe
echo.
echo Usage:
echo   compress.exe input_file output.huff
echo   decompress.exe output.huff recovered_file
echo.
