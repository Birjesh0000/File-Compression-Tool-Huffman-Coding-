#include <iostream>
#include <queue>
#include <memory>
#include <unordered_map>
#include <string>
#include <fstream>
#include <chrono>
#include <filesystem>

// HuffmanNode class
class HuffmanNode {
public:
    char character;
    int frequency;
    std::shared_ptr<HuffmanNode> left;
    std::shared_ptr<HuffmanNode> right;

    // Constructor
    HuffmanNode(char ch = '\0', int freq = 0)
        : character(ch), frequency(freq), left(nullptr), right(nullptr) {}

    // Destructor
    ~HuffmanNode() = default;
};

// Custom comparator struct for min-heap
struct CompareNode {
    bool operator()(const std::shared_ptr<HuffmanNode>& a, 
                    const std::shared_ptr<HuffmanNode>& b) const {
        return a->frequency > b->frequency;
    }
};

// BitReader class for handling bit-level file reading
class BitReader {
private:
    std::ifstream inFile;
    unsigned char buffer;
    int bitCount;
    bool eof;

public:
    // Constructor
    BitReader() : buffer(0), bitCount(0), eof(false) {}

    // Open a file for reading
    void open(const std::string& filename) {
        inFile.open(filename, std::ios::binary);
        if (!inFile.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for reading.\n";
        }
        bitCount = 8;
    }

    // Read a single bit from the file
    int readBit() {
        if (!inFile.is_open()) {
            std::cerr << "Error: File is not open. Call open() first.\n";
            return -1;
        }

        if (bitCount == 8) {
            char ch;
            if (!inFile.get(ch)) {
                eof = true;
                return -1;
            }
            buffer = static_cast<unsigned char>(ch);
            bitCount = 0;
        }

        int bit = (buffer >> (7 - bitCount)) & 1;
        bitCount++;

        return bit;
    }

    // Check if end of file reached
    bool isEof() const {
        return eof;
    }

    // Close the file
    void close() {
        if (inFile.is_open()) {
            inFile.close();
        }
    }

    // Check if file is open
    bool isOpen() const {
        return inFile.is_open();
    }
};

// HuffmanTree class
class HuffmanTree {
private:
    std::shared_ptr<HuffmanNode> root;

public:
    HuffmanTree() : root(nullptr) {}

    void buildTree(const std::unordered_map<char, int>& freqMap) {
        if (freqMap.empty()) {
            std::cout << "Error: Frequency map is empty!\n";
            return;
        }

        std::priority_queue<std::shared_ptr<HuffmanNode>, 
                           std::vector<std::shared_ptr<HuffmanNode>>, 
                           CompareNode> pq;

        for (const auto& pair : freqMap) {
            pq.push(std::make_shared<HuffmanNode>(pair.first, pair.second));
        }

        while (pq.size() > 1) {
            auto left = pq.top();
            pq.pop();
            auto right = pq.top();
            pq.pop();

            int combinedFreq = left->frequency + right->frequency;
            auto parent = std::make_shared<HuffmanNode>('\0', combinedFreq);
            parent->left = left;
            parent->right = right;

            pq.push(parent);
        }

        root = pq.top();
    }

    std::shared_ptr<HuffmanNode> getRoot() const {
        return root;
    }
};

// FileDecompressor class
class FileDecompressor {
private:
    std::unordered_map<char, int> frequencyMap;
    HuffmanTree tree;

public:
    bool readHeader(const std::string& compressedFile) {
        std::ifstream inFile(compressedFile, std::ios::binary);
        if (!inFile.is_open()) {
            std::cerr << "Error: Could not open compressed file " << compressedFile << "\n";
            return false;
        }

        frequencyMap.clear();

        unsigned char numChars;
        if (!inFile.get(reinterpret_cast<char&>(numChars))) {
            std::cerr << "Error: Could not read header.\n";
            inFile.close();
            return false;
        }

        std::cout << "\nReading Header:\n";
        std::cout << "Number of unique characters: " << static_cast<int>(numChars) << "\n";
        std::cout << "================================================\n";

        for (int i = 0; i < numChars; ++i) {
            char ch;
            unsigned char byte1, byte2, byte3, byte4;

            if (!inFile.get(ch)) {
                std::cerr << "Error: Could not read character from header.\n";
                inFile.close();
                return false;
            }

            if (!inFile.get(reinterpret_cast<char&>(byte1)) ||
                !inFile.get(reinterpret_cast<char&>(byte2)) ||
                !inFile.get(reinterpret_cast<char&>(byte3)) ||
                !inFile.get(reinterpret_cast<char&>(byte4))) {
                std::cerr << "Error: Could not read frequency from header.\n";
                inFile.close();
                return false;
            }

            int freq = (static_cast<int>(byte1) << 24) |
                      (static_cast<int>(byte2) << 16) |
                      (static_cast<int>(byte3) << 8) |
                      static_cast<int>(byte4);

            frequencyMap[ch] = freq;
            std::cout << "Character '" << ch << "' (ASCII " << static_cast<int>(static_cast<unsigned char>(ch)) 
                      << ") -> Frequency: " << freq << "\n";
        }

        std::cout << "================================================\n";
        inFile.close();
        return true;
    }

    void decompress(const std::string& compressedFile, const std::string& outputFile) {
        std::cout << "\n\nStarting Decompression Process...\n";
        std::cout << "================================================\n";

        std::cout << "\nStep 1: Reading File Header\n";
        if (!readHeader(compressedFile)) {
            std::cerr << "Error: Failed to read header.\n";
            return;
        }

        std::cout << "\nStep 2: Building Huffman Tree\n";
        tree.buildTree(frequencyMap);
        std::cout << "Huffman Tree reconstructed successfully.\n";

        std::cout << "\nStep 3: Reading Encoded Data and Decompressing\n";
        
        // Calculate header size
        int headerSize = 1;  // numChars byte
        for (int i = 0; i < static_cast<int>(frequencyMap.size()); ++i) {
            headerSize += 5;  // char + 4 bytes for frequency
        }

        // Open file and skip header
        std::ifstream skipFile(compressedFile, std::ios::binary);
        skipFile.ignore(headerSize);

        // Open output file
        std::ofstream outFile(outputFile, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open output file " << outputFile << "\n";
            skipFile.close();
            return;
        }

        // Read bits and decompress
        BitReader bitReader;
        bitReader.open(compressedFile);

        // Skip header in BitReader
        for (int i = 0; i < headerSize; ++i) {
            for (int j = 0; j < 8; ++j) {
                bitReader.readBit();
            }
        }

        auto root = tree.getRoot();
        auto currentNode = root;
        int totalCharsWritten = 0;

        while (true) {
            int bit = bitReader.readBit();
            if (bit == -1) break;

            if (bit == 0) {
                currentNode = currentNode->left;
            } else {
                currentNode = currentNode->right;
            }

            if (currentNode && currentNode->character != '\0') {
                outFile.put(currentNode->character);
                totalCharsWritten++;
                currentNode = root;
            }
        }

        outFile.close();
        bitReader.close();
        skipFile.close();

        std::cout << "Total characters written: " << totalCharsWritten << "\n";
        std::cout << "================================================\n";
        std::cout << "Decompression complete! Output file: " << outputFile << "\n";
    }
};

// Main CLI function for decompression
int main(int argc, char* argv[]) {
    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║    Huffman File Decompressor - CLI Tool         ║\n";
    std::cout << "║         Version 1.0 (Decompression)             ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";

    if (argc < 3) {
        std::cerr << "Error: Insufficient arguments.\n\n";
        std::cout << "Usage: decompress.exe <compressed_file> <output_file>\n\n";
        std::cout << "Example:\n";
        std::cout << "  decompress.exe myfile.huff myfile.txt\n\n";
        std::cout << "This will decompress 'myfile.huff' into 'myfile.txt'\n";
        return 1;
    }

    std::string compressedFile = argv[1];
    std::string outputFile = argv[2];

    std::ifstream checkFile(compressedFile);
    if (!checkFile.is_open()) {
        std::cerr << "Error: Compressed file '" << compressedFile << "' not found.\n";
        return 1;
    }
    checkFile.close();

    std::uintmax_t compressedSize = std::filesystem::file_size(compressedFile);
    std::cout << "Compressed file: " << compressedFile << "\n";
    std::cout << "Output file: " << outputFile << "\n";
    std::cout << "Compressed file size: " << compressedSize << " bytes\n\n";

    auto startTime = std::chrono::high_resolution_clock::now();

    FileDecompressor decompressor;
    decompressor.decompress(compressedFile, outputFile);

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::uintmax_t decompressedSize = std::filesystem::file_size(outputFile);

    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║           DECOMPRESSION STATISTICS              ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";

    std::cout << "Compressed size:   " << compressedSize << " bytes\n";
    std::cout << "Decompressed size: " << decompressedSize << " bytes\n";
    std::cout << "Execution time:    " << duration.count() << " ms\n\n";

    std::cout << "✓ Decompression completed successfully!\n";
    std::cout << "✓ Decompressed file saved to: " << outputFile << "\n\n";

    return 0;
}
