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

// Custom comparator struct for min-heap (lowest frequency at top)
struct CompareNode {
    bool operator()(const std::shared_ptr<HuffmanNode>& a, 
                    const std::shared_ptr<HuffmanNode>& b) const {
        return a->frequency > b->frequency;
    }
};

// BitWriter class for handling bit-level file writing
class BitWriter {
private:
    std::ofstream outFile;
    unsigned char buffer;
    int bitCount;

public:
    // Constructor
    BitWriter() : buffer(0), bitCount(0) {}

    // Open a file for writing
    void open(const std::string& filename) {
        outFile.open(filename, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for writing.\n";
        }
    }

    // Write a single bit to the buffer
    void writeBit(int bit) {
        if (!outFile.is_open()) {
            std::cerr << "Error: File is not open. Call open() first.\n";
            return;
        }

        buffer = (buffer << 1) | (bit & 1);
        bitCount++;

        if (bitCount == 8) {
            outFile.put(buffer);
            buffer = 0;
            bitCount = 0;
        }
    }

    // Close the file and handle any remaining bits (padding)
    void close() {
        if (!outFile.is_open()) {
            std::cerr << "Error: File is not open.\n";
            return;
        }

        if (bitCount > 0) {
            buffer = buffer << (8 - bitCount);
            outFile.put(buffer);
            std::cout << "Wrote " << bitCount << " bits with " << (8 - bitCount) 
                      << " padding bits.\n";
        }

        outFile.close();
        std::cout << "File closed successfully.\n";
    }

    // Check if file is open
    bool isOpen() const {
        return outFile.is_open();
    }
};

// HuffmanTree class for building and managing the Huffman Tree
class HuffmanTree {
private:
    std::shared_ptr<HuffmanNode> root;
    std::unordered_map<char, std::string> codes;

public:
    // Constructor
    HuffmanTree() : root(nullptr) {}

    // Build the Huffman Tree from character frequencies
    void buildTree(const std::unordered_map<char, int>& freqMap) {
        if (freqMap.empty()) {
            std::cout << "Error: Frequency map is empty!\n";
            return;
        }

        std::priority_queue<std::shared_ptr<HuffmanNode>, 
                           std::vector<std::shared_ptr<HuffmanNode>>, 
                           CompareNode> pq;

        for (const auto& pair : freqMap) {
            char ch = pair.first;
            int freq = pair.second;
            pq.push(std::make_shared<HuffmanNode>(ch, freq));
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

    // Helper function to recursively generate codes
    void generateCodesHelper(std::shared_ptr<HuffmanNode> node, 
                            const std::string& currentCode) {
        if (!node) return;

        if (node->character != '\0') {
            codes[node->character] = currentCode;
            return;
        }

        if (node->left) {
            generateCodesHelper(node->left, currentCode + "0");
        }

        if (node->right) {
            generateCodesHelper(node->right, currentCode + "1");
        }
    }

    // Generate Huffman codes for all characters in the tree
    void generateCodes() {
        if (!root) {
            std::cout << "Error: Tree is empty!\n";
            return;
        }

        if (root->character != '\0') {
            codes[root->character] = "0";
            return;
        }

        codes.clear();
        generateCodesHelper(root, "");
    }

    // Print the generated codes
    void printCodes() const {
        if (codes.empty()) {
            std::cout << "No codes generated. Call generateCodes() first.\n";
            return;
        }

        std::cout << "\nHuffman Codes:\n";
        std::cout << "================================================\n";
        for (const auto& pair : codes) {
            std::cout << "Character '" << pair.first << "' -> " << pair.second << std::endl;
        }
        std::cout << "================================================\n";
    }

    // Getter for the root node
    std::shared_ptr<HuffmanNode> getRoot() const {
        return root;
    }

    // Getter for the codes map
    const std::unordered_map<char, std::string>& getCodes() const {
        return codes;
    }

    // Helper function to print the tree structure
    void printTree(std::shared_ptr<HuffmanNode> node, int depth = 0) const {
        if (!node) return;

        printTree(node->right, depth + 1);

        for (int i = 0; i < depth; ++i) std::cout << "  ";
        if (node->character != '\0') {
            std::cout << "[" << node->character << "] freq: " << node->frequency << std::endl;
        } else {
            std::cout << "[*] freq: " << node->frequency << std::endl;
        }

        printTree(node->left, depth + 1);
    }
};

// FileCompressor class that integrates all components
class FileCompressor {
private:
    HuffmanTree tree;
    std::unordered_map<char, int> frequencyMap;

public:
    // Calculate character frequencies from input file
    void calculateFrequencies(const std::string& inputFile) {
        frequencyMap.clear();
        std::ifstream inFile(inputFile, std::ios::binary);

        if (!inFile.is_open()) {
            std::cerr << "Error: Could not open input file " << inputFile << "\n";
            return;
        }

        char ch;
        while (inFile.get(ch)) {
            frequencyMap[ch]++;
        }

        inFile.close();
        std::cout << "Calculated frequencies for " << frequencyMap.size() 
                  << " unique characters.\n";
    }

    // Write the frequency map as a header to the output file
    void writeHeader(std::ofstream& outFile) {
        if (!outFile.is_open()) {
            std::cerr << "Error: Output file is not open.\n";
            return;
        }

        unsigned char numChars = static_cast<unsigned char>(frequencyMap.size());
        outFile.put(numChars);

        std::cout << "\nWriting Header:\n";
        std::cout << "Number of unique characters: " << static_cast<int>(numChars) << "\n";
        std::cout << "================================================\n";

        for (const auto& pair : frequencyMap) {
            char ch = pair.first;
            int freq = pair.second;

            outFile.put(ch);

            unsigned char byte1 = (freq >> 24) & 0xFF;
            unsigned char byte2 = (freq >> 16) & 0xFF;
            unsigned char byte3 = (freq >> 8) & 0xFF;
            unsigned char byte4 = freq & 0xFF;

            outFile.put(byte1);
            outFile.put(byte2);
            outFile.put(byte3);
            outFile.put(byte4);

            std::cout << "Character '" << ch << "' (ASCII " << static_cast<int>(static_cast<unsigned char>(ch)) 
                      << ") -> Frequency: " << freq << "\n";
        }

        std::cout << "================================================\n";
    }

    // Main compression method
    void compress(const std::string& inputFile, const std::string& outputFile) {
        std::cout << "\n\nStarting Compression Process...\n";
        std::cout << "================================================\n";

        std::cout << "\nStep 1: Calculating Frequencies\n";
        calculateFrequencies(inputFile);

        if (frequencyMap.empty()) {
            std::cerr << "Error: Input file is empty or could not be read.\n";
            return;
        }

        std::cout << "\nStep 2: Building Huffman Tree\n";
        tree.buildTree(frequencyMap);
        std::cout << "Huffman Tree built successfully.\n";

        std::cout << "\nStep 3: Generating Huffman Codes\n";
        tree.generateCodes();
        tree.printCodes();

        std::cout << "\nStep 4: Writing File Header\n";
        std::ofstream outFile(outputFile, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open output file " << outputFile << "\n";
            return;
        }

        writeHeader(outFile);
        outFile.close();

        std::cout << "\nStep 5: Writing Encoded Data\n";
        BitWriter bitWriter;
        bitWriter.open(outputFile);

        if (!bitWriter.isOpen()) {
            std::cerr << "Error: BitWriter failed to open file.\n";
            return;
        }

        std::ifstream inFile(inputFile, std::ios::binary);
        if (!inFile.is_open()) {
            std::cerr << "Error: Could not open input file for reading.\n";
            return;
        }

        const auto& codes = tree.getCodes();
        int bitsWritten = 0;
        char ch;

        while (inFile.get(ch)) {
            if (codes.find(ch) != codes.end()) {
                std::string code = codes.at(ch);
                for (char bit : code) {
                    bitWriter.writeBit(bit - '0');
                    bitsWritten++;
                }
            }
        }

        inFile.close();
        bitWriter.close();

        std::cout << "Total bits written: " << bitsWritten << "\n";
        std::cout << "================================================\n";
        std::cout << "Compression complete! Output file: " << outputFile << "\n";
    }
};

// Main CLI function
int main(int argc, char* argv[]) {
    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║     Huffman File Compressor - CLI Tool          ║\n";
    std::cout << "║          Version 1.0 (Compression)              ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";

    if (argc < 3) {
        std::cerr << "Error: Insufficient arguments.\n\n";
        std::cout << "Usage: compress.exe <input_file> <output_file>\n\n";
        std::cout << "Example:\n";
        std::cout << "  compress.exe myfile.txt myfile.huff\n\n";
        std::cout << "This will compress 'myfile.txt' into 'myfile.huff'\n";
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    std::ifstream checkFile(inputFile);
    if (!checkFile.is_open()) {
        std::cerr << "Error: Input file '" << inputFile << "' not found.\n";
        return 1;
    }
    checkFile.close();

    std::uintmax_t originalSize = std::filesystem::file_size(inputFile);
    std::cout << "Input file: " << inputFile << "\n";
    std::cout << "Output file: " << outputFile << "\n";
    std::cout << "Original file size: " << originalSize << " bytes\n\n";

    auto startTime = std::chrono::high_resolution_clock::now();

    FileCompressor compressor;
    compressor.compress(inputFile, outputFile);

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::uintmax_t compressedSize = std::filesystem::file_size(outputFile);

    double compressionRatio = (1.0 - static_cast<double>(compressedSize) / static_cast<double>(originalSize)) * 100.0;

    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║            COMPRESSION STATISTICS               ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";

    std::cout << "Original size:     " << originalSize << " bytes\n";
    std::cout << "Compressed size:   " << compressedSize << " bytes\n";
    std::cout << "Space saved:       " << (originalSize - compressedSize) << " bytes\n";
    std::cout << "Compression ratio: " << compressionRatio << "%\n";
    std::cout << "Execution time:    " << duration.count() << " ms\n\n";

    std::cout << "✓ Compression completed successfully!\n";
    std::cout << "✓ Compressed file saved to: " << outputFile << "\n\n";

    return 0;
}
