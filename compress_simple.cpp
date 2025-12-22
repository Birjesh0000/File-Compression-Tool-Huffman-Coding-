#include <iostream>
#include <fstream>
#include <queue>
#include <memory>
#include <unordered_map>
#include <string>

struct Node {
    char ch;
    int freq;
    std::shared_ptr<Node> left, right;
    Node(char c = '\0', int f = 0) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

struct CompareNode {
    bool operator()(const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) const {
        return a->freq > b->freq;
    }
};

void generateCodes(const std::shared_ptr<Node>& node, const std::string& code,
                  std::unordered_map<char, std::string>& codes) {
    if (!node) return;
    if (node->ch != '\0') {
        codes[node->ch] = code.empty() ? "0" : code;
        return;
    }
    if (node->left) generateCodes(node->left, code + "0", codes);
    if (node->right) generateCodes(node->right, code + "1", codes);
}

void writeHeader(std::ofstream& out, const std::unordered_map<char, int>& freq, int totalChars) {
    unsigned char numChars = freq.size();
    out.put(numChars);
    
    unsigned char b1 = (totalChars >> 24) & 0xFF;
    unsigned char b2 = (totalChars >> 16) & 0xFF;
    unsigned char b3 = (totalChars >> 8) & 0xFF;
    unsigned char b4 = totalChars & 0xFF;
    out.put(b1);
    out.put(b2);
    out.put(b3);
    out.put(b4);
    
    for (const auto& [ch, f] : freq) {
        out.put(ch);
        out.put((f >> 24) & 0xFF);
        out.put((f >> 16) & 0xFF);
        out.put((f >> 8) & 0xFF);
        out.put(f & 0xFF);
    }
}

void compress(const std::string& inFile, const std::string& outFile) {
    std::cout << "Compressing " << inFile << "..." << std::endl;

    // Read file and calculate frequencies
    std::unordered_map<char, int> freq;
    std::ifstream in(inFile, std::ios::binary);
    if (!in) {
        std::cerr << "Error: Cannot open input file." << std::endl;
        return;
    }
    char ch;
    int totalChars = 0;
    while (in.get(ch)) {
        freq[ch]++;
        totalChars++;
    }
    in.close();

    if (freq.empty()) {
        std::cerr << "Error: Input file is empty." << std::endl;
        return;
    }

    // Build Huffman tree
    std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, CompareNode> pq;
    for (const auto& [c, f] : freq) pq.push(std::make_shared<Node>(c, f));
    while (pq.size() > 1) {
        auto left = pq.top(); pq.pop();
        auto right = pq.top(); pq.pop();
        auto parent = std::make_shared<Node>('\0', left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }
    auto root = pq.top();

    // Generate codes
    std::unordered_map<char, std::string> codes;
    generateCodes(root, "", codes);

    // Write header
    std::ofstream out(outFile, std::ios::binary);
    if (!out) {
        std::cerr << "Error: Cannot open output file." << std::endl;
        return;
    }
    writeHeader(out, freq, totalChars);
    out.close();

    // Write encoded data
    out.open(outFile, std::ios::binary | std::ios::app);
    unsigned char buffer = 0, bitCount = 0;
    in.open(inFile, std::ios::binary);
    while (in.get(ch)) {
        for (char bit : codes[ch]) {
            buffer = (buffer << 1) | (bit - '0');
            bitCount++;
            if (bitCount == 8) {
                out.put(buffer);
                buffer = 0;
                bitCount = 0;
            }
        }
    }
    if (bitCount > 0) out.put(buffer << (8 - bitCount));
    in.close();
    out.close();

    std::cout << "Compression complete!" << std::endl;
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: compress.exe <input_file> <output_file>" << std::endl;
        return 1;
    }
    std::ifstream test(argv[1]);
    if (!test) {
        std::cerr << "Error: Input file not found." << std::endl;
        return 1;
    }
    test.close();
    compress(argv[1], argv[2]);
    std::cout << "File saved to: " << argv[2] << std::endl;
    return 0;
}
