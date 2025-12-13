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
        return a->frequency > b->frequency;  // Min-heap: smaller frequency has higher priority
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

        // Shift the bit into the buffer (MSB first)
        buffer = (buffer << 1) | (bit & 1);
        bitCount++;

        // If buffer is full (8 bits), write the byte to file
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

        // If there are remaining bits, pad with zeros and write
        if (bitCount > 0) {
            buffer = buffer << (8 - bitCount);  // Pad with zeros on the right
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
        bitCount = 8;  // Start with empty buffer
    }

    // Read a single bit from the file
    int readBit() {
        if (!inFile.is_open()) {
            std::cerr << "Error: File is not open. Call open() first.\n";
            return -1;
        }

        // If buffer is empty, read a new byte
        if (bitCount == 8) {
            char ch;
            if (!inFile.get(ch)) {
                eof = true;
                return -1;
            }
            buffer = static_cast<unsigned char>(ch);
            bitCount = 0;
        }

        // Extract the MSB from the buffer
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

        // Write number of unique characters (1 byte)
        unsigned char numChars = static_cast<unsigned char>(frequencyMap.size());
        outFile.put(numChars);

        std::cout << "\nWriting Header:\n";
        std::cout << "Number of unique characters: " << static_cast<int>(numChars) << "\n";
        std::cout << "================================================\n";

        // Write each character and its frequency (char + 4-byte int)
        for (const auto& pair : frequencyMap) {
            char ch = pair.first;
            int freq = pair.second;

            // Write character (1 byte)
            outFile.put(ch);

            // Write frequency (4 bytes, big-endian)
            unsigned char byte1 = (freq >> 24) & 0xFF;
            unsigned char byte2 = (freq >> 16) & 0xFF;
            unsigned char byte3 = (freq >> 8) & 0xFF;
            unsigned char byte4 = freq & 0xFF;

            outFile.put(byte1);
            outFile.put(byte2);
            outFile.put(byte3);
            outFile.put(byte4);

            std::cout << "Character '" << ch << "' (ASCII " << static_cast<int>(ch) 
                      << ") -> Frequency: " << freq << "\n";
        }

        std::cout << "================================================\n";
    }

    // Main compression method
    void compress(const std::string& inputFile, const std::string& outputFile) {
        std::cout << "\n\nStarting Compression Process...\n";
        std::cout << "================================================\n";

        // Step 1: Calculate frequencies
        std::cout << "\nStep 1: Calculating Frequencies\n";
        calculateFrequencies(inputFile);

        if (frequencyMap.empty()) {
            std::cerr << "Error: Input file is empty or could not be read.\n";
            return;
        }

        // Step 2: Build Huffman Tree
        std::cout << "\nStep 2: Building Huffman Tree\n";
        tree.buildTree(frequencyMap);
        std::cout << "Huffman Tree built successfully.\n";

        // Step 3: Generate Huffman Codes
        std::cout << "\nStep 3: Generating Huffman Codes\n";
        tree.generateCodes();
        tree.printCodes();

        // Step 4: Open output file and write header
        std::cout << "\nStep 4: Writing File Header\n";
        std::ofstream outFile(outputFile, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open output file " << outputFile << "\n";
            return;
        }

        writeHeader(outFile);
        outFile.close();

        // Step 5: Write encoded data using BitWriter
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

// FileDecompressor class for decompressing Huffman-encoded files
class FileDecompressor {
private:
    std::unordered_map<char, int> frequencyMap;
    HuffmanTree tree;

public:
    // Read the header from the compressed file
    bool readHeader(const std::string& compressedFile) {
        std::ifstream inFile(compressedFile, std::ios::binary);
        if (!inFile.is_open()) {
            std::cerr << "Error: Could not open compressed file " << compressedFile << "\n";
            return false;
        }

        frequencyMap.clear();

        // Read number of unique characters
        unsigned char numChars;
        if (!inFile.get(reinterpret_cast<char&>(numChars))) {
            std::cerr << "Error: Could not read header.\n";
            inFile.close();
            return false;
        }

        std::cout << "\nReading Header:\n";
        std::cout << "Number of unique characters: " << static_cast<int>(numChars) << "\n";
        std::cout << "================================================\n";

        // Read each character and its frequency
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

            // Reconstruct the frequency (big-endian)
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

    // Main decompression method
    void decompress(const std::string& compressedFile, const std::string& outputFile) {
        std::cout << "\n\nStarting Decompression Process...\n";
        std::cout << "================================================\n";

        // Step 1: Read header
        std::cout << "\nStep 1: Reading File Header\n";
        if (!readHeader(compressedFile)) {
            std::cerr << "Error: Failed to read header.\n";
            return;
        }

        // Step 2: Build Huffman Tree
        std::cout << "\nStep 2: Building Huffman Tree\n";
        tree.buildTree(frequencyMap);
        std::cout << "Huffman Tree reconstructed successfully.\n";

        // Step 3: Read encoded data and decompress
        std::cout << "\nStep 3: Reading Encoded Data and Decompressing\n";
        BitReader bitReader;
        bitReader.open(compressedFile);

        if (!bitReader.isOpen()) {
            std::cerr << "Error: BitReader failed to open file.\n";
            return;
        }

        // Skip the header in the BitReader
        unsigned char numChars;
        std::ifstream skipFile(compressedFile, std::ios::binary);
        skipFile.get(reinterpret_cast<char&>(numChars));
        for (int i = 0; i < static_cast<int>(numChars); ++i) {
            skipFile.get();  // char
            skipFile.get();  // byte1
            skipFile.get();  // byte2
            skipFile.get();  // byte3
            skipFile.get();  // byte4
        }
        std::streampos headerEnd = skipFile.tellg();
        skipFile.close();

        // Open BitReader and skip to encoded data
        bitReader.close();
        bitReader.open(compressedFile);
        // Read bits to skip header
        for (std::streampos i = 0; i < headerEnd; ++i) {
            for (int j = 0; j < 8; ++j) {
                bitReader.readBit();
            }
        }

        // Open output file for writing
        std::ofstream outFile(outputFile, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open output file " << outputFile << "\n";
            bitReader.close();
            return;
        }

        // Traverse the tree using the bit stream
        auto root = tree.getRoot();
        auto currentNode = root;
        int totalCharsWritten = 0;

        while (true) {
            int bit = bitReader.readBit();
            if (bit == -1) break;  // End of file

            // Traverse the tree based on the bit
            if (bit == 0) {
                currentNode = currentNode->left;
            } else {
                currentNode = currentNode->right;
            }

            // If we reach a leaf node, write the character
            if (currentNode && currentNode->character != '\0') {
                outFile.put(currentNode->character);
                totalCharsWritten++;
                currentNode = root;  // Reset to root for next character
            }
        }

        outFile.close();
        bitReader.close();

        std::cout << "Total characters written: " << totalCharsWritten << "\n";
        std::cout << "================================================\n";
        std::cout << "Decompression complete! Output file: " << outputFile << "\n";
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

        // Create a priority queue with the custom comparator
        std::priority_queue<std::shared_ptr<HuffmanNode>, 
                           std::vector<std::shared_ptr<HuffmanNode>>, 
                           CompareNode> pq;

        // Create leaf nodes for each character and add to priority queue
        for (const auto& pair : freqMap) {
            char ch = pair.first;
            int freq = pair.second;
            pq.push(std::make_shared<HuffmanNode>(ch, freq));
        }

        // Build the tree by combining nodes
        while (pq.size() > 1) {
            // Pop two nodes with smallest frequencies
            auto left = pq.top();
            pq.pop();
            auto right = pq.top();
            pq.pop();

            // Create a new internal node with combined frequency
            int combinedFreq = left->frequency + right->frequency;
            auto parent = std::make_shared<HuffmanNode>('\0', combinedFreq);
            parent->left = left;
            parent->right = right;

            // Push the new node back to the priority queue
            pq.push(parent);
        }

        // The last remaining node is the root
        root = pq.top();
    }

    // Helper function to recursively generate codes
    void generateCodesHelper(std::shared_ptr<HuffmanNode> node, 
                            const std::string& currentCode) {
        if (!node) return;

        // If it's a leaf node (character node), store the code
        if (node->character != '\0') {
            codes[node->character] = currentCode;
            return;
        }

        // Traverse left with appending '0'
        if (node->left) {
            generateCodesHelper(node->left, currentCode + "0");
        }

        // Traverse right with appending '1'
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

        // Special case: if root is a leaf node (only one character)
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

    // Helper function to print the tree structure (in-order traversal)
    void printTree(std::shared_ptr<HuffmanNode> node, int depth = 0) const {
        if (!node) return;

        // Print right subtree
        printTree(node->right, depth + 1);

        // Print current node
        for (int i = 0; i < depth; ++i) std::cout << "  ";
        if (node->character != '\0') {
            std::cout << "[" << node->character << "] freq: " << node->frequency << std::endl;
        } else {
            std::cout << "[*] freq: " << node->frequency << std::endl;
        }

        // Print left subtree
        printTree(node->left, depth + 1);
    }
};


// Main CLI function
int main(int argc, char* argv[]) {
    // Display welcome banner
    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║     Huffman File Compressor - CLI Tool          ║\n";
    std::cout << "║          Version 1.0 (Compression)              ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";

    // Check command-line arguments
    if (argc < 3) {
        std::cerr << "Error: Insufficient arguments.\n\n";
        std::cout << "Usage: huffman_node <input_file> <output_file>\n\n";
        std::cout << "Example:\n";
        std::cout << "  huffman_node myfile.txt myfile.huff\n\n";
        std::cout << "This will compress 'myfile.txt' into 'myfile.huff'\n";
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    // Verify input file exists
    std::ifstream checkFile(inputFile);
    if (!checkFile.is_open()) {
        std::cerr << "Error: Input file '" << inputFile << "' not found.\n";
        return 1;
    }
    checkFile.close();

    // Get original file size
    std::uintmax_t originalSize = std::filesystem::file_size(inputFile);
    std::cout << "Input file: " << inputFile << "\n";
    std::cout << "Output file: " << outputFile << "\n";
    std::cout << "Original file size: " << originalSize << " bytes\n\n";

    // Start timer
    auto startTime = std::chrono::high_resolution_clock::now();

    // Create compressor and compress
    FileCompressor compressor;
    compressor.compress(inputFile, outputFile);

    // End timer
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // Get compressed file size
    std::uintmax_t compressedSize = std::filesystem::file_size(outputFile);

    // Calculate compression ratio
    double compressionRatio = (1.0 - static_cast<double>(compressedSize) / static_cast<double>(originalSize)) * 100.0;

    // Display statistics
    std::cout << "\n╔════════════════════════════════════════════════╗\n";
    std::cout << "║            COMPRESSION STATISTICS               ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";

    std::cout << "Original size:    " << originalSize << " bytes\n";
    std::cout << "Compressed size:  " << compressedSize << " bytes\n";
    std::cout << "Space saved:      " << (originalSize - compressedSize) << " bytes\n";
    std::cout << "Compression ratio: " << compressionRatio << "%\n";
    std::cout << "Execution time:   " << duration.count() << " ms\n\n";

    std::cout << "✓ Compression completed successfully!\n";
    std::cout << "✓ Compressed file saved to: " << outputFile << "\n\n";

    return 0;
}


