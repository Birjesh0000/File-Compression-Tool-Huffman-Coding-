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

class BitWriter {
private:
    std::ofstream outFile;
    unsigned char buffer;
    int bitCount;
    bool append;

public:
    BitWriter(bool append_mode = false) : buffer(0), bitCount(0), append(append_mode) {}

    void open(const std::string& filename) {
        if (append) {
            outFile.open(filename, std::ios::binary | std::ios::app);
        } else {
            outFile.open(filename, std::ios::binary);
        }
    }

    void writeBit(int bit) {
        buffer = (buffer << 1) | (bit & 1);
        bitCount++;

        if (bitCount == 8) {
            outFile.put(buffer);
            buffer = 0;
            bitCount = 0;
        }
    }

    void close() {
        if (bitCount > 0) {
            buffer = buffer << (8 - bitCount);
            outFile.put(buffer);
        }
        outFile.close();
    }

    bool isOpen() const {
        return outFile.is_open();
    }
};

class HuffmanTree {
private:
    std::shared_ptr<HuffmanNode> root;
    std::unordered_map<char, std::string> codes;

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

    void generateCodes() {
        if (!root) return;

        if (root->character != '\0') {
            codes[root->character] = "0";
            return;
        }

        codes.clear();
        generateCodesHelper(root, "");
    }

    const std::unordered_map<char, std::string>& getCodes() const {
        return codes;
    }

    std::shared_ptr<HuffmanNode> getRoot() const {
        return root;
    }
};

class FileCompressor {
private:
    HuffmanTree tree;
    std::unordered_map<char, int> frequencyMap;

public:
    void calculateFrequencies(const std::string& inputFile) {
        frequencyMap.clear();
        std::ifstream inFile(inputFile, std::ios::binary);

        if (!inFile.is_open()) return;

        char ch;
        while (inFile.get(ch)) {
            frequencyMap[ch]++;
        }
        inFile.close();
    }

    void writeHeader(std::ofstream& outFile) {
        unsigned char numChars = static_cast<unsigned char>(frequencyMap.size());
        outFile.put(numChars);

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
        }
    }

    void compress(const std::string& inputFile, const std::string& outputFile) {
        std::cout << "Compressing " << inputFile << "..." << std::endl;

        calculateFrequencies(inputFile);

        if (frequencyMap.empty()) {
            std::cerr << "Error: Input file is empty." << std::endl;
            return;
        }

        tree.buildTree(frequencyMap);
        tree.generateCodes();

        // Write header
        std::ofstream outFile(outputFile, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open output file." << std::endl;
            return;
        }

        writeHeader(outFile);
        outFile.close();

        // Write data
        BitWriter bitWriter(true);  // append mode
        bitWriter.open(outputFile);

        if (!bitWriter.isOpen()) {
            std::cerr << "Error: BitWriter failed to open file." << std::endl;
            return;
        }

        std::ifstream inFile(inputFile, std::ios::binary);
        if (!inFile.is_open()) {
            std::cerr << "Error: Could not open input file for reading." << std::endl;
            return;
        }

        const auto& codes = tree.getCodes();
        char ch;

        while (inFile.get(ch)) {
            if (codes.find(ch) != codes.end()) {
                std::string code = codes.at(ch);
                for (char bit : code) {
                    bitWriter.writeBit(bit - '0');
                }
            }
        }

        inFile.close();
        bitWriter.close();

        std::cout << "Compression complete!" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: compress.exe <input_file> <output_file>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    std::ifstream checkFile(inputFile);
    if (!checkFile.is_open()) {
        std::cerr << "Error: Input file not found." << std::endl;
        return 1;
    }
    checkFile.close();

    FileCompressor compressor;
    compressor.compress(inputFile, outputFile);

    std::cout << "File saved to: " << outputFile << std::endl;

    return 0;
}
