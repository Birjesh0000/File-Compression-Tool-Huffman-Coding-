#!/bin/bash
# Huffman Compressor/Decompressor Build Script
# This script compiles the project with available C++ compiler

echo ""
echo "Huffman File Compressor/Decompressor - Build Script"
echo "===================================================="
echo ""

# Try to find and use available C++ compiler
if command -v clang++ &> /dev/null; then
    echo "Found clang++. Compiling..."
    clang++ -std=c++17 -o compress compress_fixed.cpp
    clang++ -std=c++17 -o decompress decompress_fixed.cpp
elif command -v g++ &> /dev/null; then
    echo "Found g++. Compiling..."
    g++ -std=c++17 -o compress compress_fixed.cpp
    g++ -std=c++17 -o decompress decompress_fixed.cpp
else
    echo "Error: No C++ compiler found!"
    echo "Please install one of: GCC or Clang"
    exit 1
fi

echo ""
echo "Build completed!"
echo "Executables: compress, decompress"
echo ""
echo "Usage:"
echo "  ./compress input_file output.huff"
echo "  ./decompress output.huff recovered_file"
echo ""
