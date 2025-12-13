# Huffman File Compressor/Decompressor

A complete implementation of Huffman coding for file compression and decompression in C++.

## Project Structure

```
Huffman/
├── huffman_node.cpp        # Complete all-in-one implementation
├── compress.cpp            # Compression program (standalone)
├── decompress.cpp          # Decompression program (standalone)
├── build.bat               # Build script for Windows
├── build.sh                # Build script for Linux/macOS
└── README.md               # This file
```

## Features

### Compression (`compress.cpp`)
- **Frequency Analysis**: Scans input file and calculates character frequencies
- **Huffman Tree Construction**: Builds optimal binary tree using min-heap priority queue
- **Code Generation**: Generates variable-length binary codes for each character
- **Bit-Level Encoding**: Uses BitWriter for efficient binary file I/O
- **File Header**: Stores frequency map for decompression
- **Statistics**: Reports compression ratio, space saved, and execution time

### Decompression (`decompress.cpp`)
- **Header Parsing**: Reads character frequencies from compressed file
- **Tree Reconstruction**: Rebuilds the Huffman tree from frequencies
- **Bit-Level Decoding**: Uses BitReader to reconstruct original data
- **Perfect Recovery**: Recovers original file exactly
- **Performance Metrics**: Reports decompression time and file sizes

## Building the Project

### Windows (with available C++ compiler)
```cmd
cd S:\Huffman
build.bat
```

### Linux/macOS
```bash
cd Huffman
chmod +x build.sh
./build.sh
```

### Manual Compilation

**Using MSVC (Windows):**
```cmd
cl /std:latest /EHsc compress.cpp /Fe:compress.exe
cl /std:latest /EHsc decompress.cpp /Fe:decompress.exe
```

**Using GCC:**
```bash
g++ -std=c++17 -o compress compress.cpp
g++ -std=c++17 -o decompress decompress.cpp
```

**Using Clang:**
```bash
clang++ -std=c++17 -o compress compress.cpp
clang++ -std=c++17 -o decompress decompress.cpp
```

## Usage

### Compressing a File
```cmd
compress.exe myfile.txt myfile.huff
```

Output example:
```
╔════════════════════════════════════════════════╗
║     Huffman File Compressor - CLI Tool          ║
║          Version 1.0 (Compression)              ║
╚════════════════════════════════════════════════╝

Input file: myfile.txt
Output file: myfile.huff
Original file size: 1024 bytes

[Compression process...]

╔════════════════════════════════════════════════╗
║            COMPRESSION STATISTICS               ║
╚════════════════════════════════════════════════╝

Original size:     1024 bytes
Compressed size:   542 bytes
Space saved:       482 bytes
Compression ratio: 47.07%
Execution time:    5 ms

✓ Compression completed successfully!
✓ Compressed file saved to: myfile.huff
```

### Decompressing a File
```cmd
decompress.exe myfile.huff myfile_recovered.txt
```

Output example:
```
╔════════════════════════════════════════════════╗
║    Huffman File Decompressor - CLI Tool         ║
║         Version 1.0 (Decompression)             ║
╚════════════════════════════════════════════════╝

Compressed file: myfile.huff
Output file: myfile_recovered.txt
Compressed file size: 542 bytes

[Decompression process...]

╔════════════════════════════════════════════════╝
║           DECOMPRESSION STATISTICS              ║
╚════════════════════════════════════════════════╝

Compressed size:   542 bytes
Decompressed size: 1024 bytes
Execution time:    3 ms

✓ Decompression completed successfully!
✓ Decompressed file saved to: myfile_recovered.txt
```

## File Format

### Compressed File Structure

```
[Header][Encoded Data]

Header Format:
[# of unique chars (1 byte)][char1 (1 byte)][freq1 (4 bytes)]...[charN (1 byte)][freqN (4 bytes)]

Encoded Data:
Binary-encoded content using Huffman codes
```

Example: For "aaabbbccccdddddd"
- Header: `3` + `a,5` + `b,3` + `c,4` + `d,6`
- Data: Binary stream of Huffman codes

## Algorithm Overview

### Huffman Coding Steps

1. **Frequency Analysis**
   - Count occurrence of each character in input file
   - Store in `unordered_map<char, int>`

2. **Build Huffman Tree**
   - Create leaf nodes for each character with its frequency
   - Use min-heap priority queue (CompareNode struct)
   - Repeatedly combine two smallest-frequency nodes
   - Continue until single root remains

3. **Generate Codes**
   - Traverse tree from root to leaf
   - Append '0' for left, '1' for right
   - Store in `unordered_map<char, string>`

4. **Encode Data**
   - Read input file character by character
   - Output corresponding Huffman code bits
   - Use BitWriter to pack bits into bytes

5. **Write Header**
   - Store frequency map at beginning of output
   - Allows decompressor to reconstruct tree

6. **Decompress (Reverse)**
   - Read header to reconstruct frequency map
   - Rebuild Huffman tree
   - Traverse tree using bit stream
   - Output characters when reaching leaves

## Performance Characteristics

| Aspect | Complexity |
|--------|-----------|
| Frequency Analysis | O(n) |
| Tree Building | O(k log k) |
| Code Generation | O(k) |
| Compression | O(n) |
| Decompression | O(n) |
| Space (Tree) | O(k) |

Where:
- n = input file size
- k = number of unique characters

## Compression Ratio

Compression ratio depends on:
- **Character distribution**: More skewed = better compression
- **File type**: Text files compress well; binary files may not
- **Alphabet size**: Smaller alphabets generally compress better

Example results:
- Text files: 40-60% reduction
- English text: 50-70% reduction
- Binary files: 0-20% reduction (or expansion)

## Key Classes

### HuffmanNode
- Core data structure for tree nodes
- Stores character, frequency, left/right pointers

### CompareNode
- Custom comparator for priority queue
- Creates min-heap based on frequency

### HuffmanTree
- Builds and manages Huffman tree
- Generates variable-length codes
- Supports tree traversal

### BitWriter
- Writes individual bits to file
- Buffers bits into bytes
- Handles padding for partial bytes

### BitReader
- Reads individual bits from file
- Reverse of BitWriter
- Maintains bit buffer state

### FileCompressor
- Orchestrates compression process
- Calculates frequencies
- Writes headers
- Integrates all components

### FileDecompressor
- Orchestrates decompression process
- Parses headers
- Reconstructs data

## Error Handling

The programs handle:
- File not found errors
- File read/write errors
- Invalid command-line arguments
- Empty input files
- Corrupted compressed files (limited)

## Limitations

- Single-threaded
- Character encoding: ASCII/Extended ASCII only
- Maximum file size: System dependent (typically 2GB+ on modern systems)
- Unicode: Not directly supported (future enhancement)

## Future Enhancements

1. **Unicode Support**: Handle UTF-8 encoded files
2. **Multi-threading**: Parallel compression for large files
3. **Streaming**: Compress files larger than available RAM
4. **Error Correction**: Add checksums/CRC for validation
5. **Multiple Algorithms**: Support RLE, LZ77, etc.
6. **GUI**: Visual interface for drag-and-drop compression
7. **Archive Format**: Support multiple files in one archive

## Testing

### Create Test File
```powershell
@"
The quick brown fox jumps over the lazy dog. This is a test file for Huffman compression.
The quick brown fox jumps over the lazy dog. This is a test file for Huffman compression.
"@ | Set-Content test.txt
```

### Compress and Decompress
```cmd
compress.exe test.txt test.huff
decompress.exe test.huff test_recovered.txt
```

### Verify (Linux/macOS)
```bash
diff test.txt test_recovered.txt && echo "Files match!"
```

### Verify (Windows)
```cmd
fc test.txt test_recovered.txt
```

## Author Notes

This implementation demonstrates:
- Priority queue (heap) data structures
- Binary tree algorithms
- Bit manipulation techniques
- File I/O operations
- Memory management with smart pointers
- Standard Template Library (STL) usage
- Object-oriented design
- Performance optimization

## License

Public Domain - Free to use for educational and commercial purposes.

## References

- Huffman, D. A. (1952). "A Method for the Construction of Minimum-Redundancy Codes"
- Wikipedia: Huffman Coding
- Standard C++ Reference: cppreference.com

---

**Version**: 1.0  
**Language**: C++17  
**Date**: December 13, 2025
