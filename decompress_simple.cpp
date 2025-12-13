#include <iostream>
#include <fstream>
#include <queue>
#include <memory>
#include <unordered_map>
#include <string>
#include <chrono>

class HuffmanNode {
public:
    char character;
    int frequency;
    std::shared_ptr<HuffmanNode> left;
    std::shared_ptr<HuffmanNode> right;

    HuffmanNode(char ch = '\0', int freq = 0)
        : character(ch), frequency(freq), left(nullptr), right(nullptr) {}
};

struct CompareNode {
    bool operator()(const std::shared_ptr<HuffmanNode>& a, 
                    const std::shared_ptr<HuffmanNode>& b) const {
        return a->frequency > b->frequency;
    }
};

class BitReader {
private:
    std::ifstream inFile;
    unsigned char buffer;
    int bitCount;
    bool eof;

public:
    BitReader() : buffer(0), bitCount(0), eof(false) {}

    void open(const std::string& filename) {
        inFile.open(filename, std::ios::binary);
        bitCount = 8;
    }

    int readBit() {
        if (!inFile.is_open()) return -1;

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

    bool isEof() const {
        return eof;
    }

    void close() {
        if (inFile.is_open()) {
            inFile.close();
        }
    }

    bool isOpen() const {
        return inFile.is_open();
    }
};

class HuffmanTree {
private:
    std::shared_ptr<HuffmanNode> root;

public:
    HuffmanTree() : root(nullptr) {}

    void buildTree(const std::unordered_map<char, int>& freqMap) {
        if (freqMap.empty()) return;

        std::priority_queue<std::shared_ptr<HuffmanNode>, 
                           std::vector<std::shared_ptr<HuffmanNode>>, 
                           CompareNode> pq;

        for (const auto& pair : freqMap) {
            pq.push(std::make_shared<HuffmanNode>(pair.first, pair.second));
        }

        while (pq.size() > 1) {
            auto left = pq.top(); pq.pop();
            auto right = pq.top(); pq.pop();

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

class FileDecompressor {
private:
    std::unordered_map<char, int> frequencyMap;
    HuffmanTree tree;

public:
    bool readHeader(const std::string& compressedFile) {
        std::ifstream inFile(compressedFile, std::ios::binary);
        if (!inFile.is_open()) {
            std::cerr << "Error: Could not open compressed file." << std::endl;
            return false;
        }

        frequencyMap.clear();

        unsigned char numChars;
        if (!inFile.get(reinterpret_cast<char&>(numChars))) {
            std::cerr << "Error: Could not read header." << std::endl;
            inFile.close();
            return false;
        }

        for (int i = 0; i < numChars; ++i) {
            char ch;
            unsigned char byte1, byte2, byte3, byte4;

            if (!inFile.get(ch)) {
                std::cerr << "Error: Could not read character from header." << std::endl;
                inFile.close();
                return false;
            }

            if (!inFile.get(reinterpret_cast<char&>(byte1)) ||
                !inFile.get(reinterpret_cast<char&>(byte2)) ||
                !inFile.get(reinterpret_cast<char&>(byte3)) ||
                !inFile.get(reinterpret_cast<char&>(byte4))) {
                std::cerr << "Error: Could not read frequency from header." << std::endl;
                inFile.close();
                return false;
            }

            int freq = (static_cast<int>(byte1) << 24) |
                      (static_cast<int>(byte2) << 16) |
                      (static_cast<int>(byte3) << 8) |
                      static_cast<int>(byte4);

            frequencyMap[ch] = freq;
        }

        inFile.close();
        return true;
    }

    void decompress(const std::string& compressedFile, const std::string& outputFile) {
        std::cout << "Decompressing " << compressedFile << "..." << std::endl;

        if (!readHeader(compressedFile)) {
            std::cerr << "Error: Failed to read header." << std::endl;
            return;
        }

        tree.buildTree(frequencyMap);

        // Calculate header size
        int headerSize = 1;
        for (int i = 0; i < static_cast<int>(frequencyMap.size()); ++i) {
            headerSize += 5;
        }

        // Open file and skip header
        std::ifstream skipFile(compressedFile, std::ios::binary);
        skipFile.ignore(headerSize);

        // Open output file
        std::ofstream outFile(outputFile, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open output file." << std::endl;
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

        std::cout << "Decompression complete!" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: decompress.exe <compressed_file> <output_file>" << std::endl;
        return 1;
    }

    std::string compressedFile = argv[1];
    std::string outputFile = argv[2];

    std::ifstream checkFile(compressedFile);
    if (!checkFile.is_open()) {
        std::cerr << "Error: Compressed file not found." << std::endl;
        return 1;
    }
    checkFile.close();

    FileDecompressor decompressor;
    decompressor.decompress(compressedFile, outputFile);

    std::cout << "File saved to: " << outputFile << std::endl;

    return 0;
}
