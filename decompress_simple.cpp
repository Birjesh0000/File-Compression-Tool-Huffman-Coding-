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

void decompress(const std::string& inFile, const std::string& outFile) {
    std::cout << "Decompressing " << inFile << "..." << std::endl;

    std::ifstream in(inFile, std::ios::binary);
    if (!in) {
        std::cerr << "Error: Cannot open compressed file." << std::endl;
        return;
    }

    // Read header
    unsigned char numChars;
    in.get(reinterpret_cast<char&>(numChars));
    
    unsigned char b1, b2, b3, b4;
    in.get(reinterpret_cast<char&>(b1));
    in.get(reinterpret_cast<char&>(b2));
    in.get(reinterpret_cast<char&>(b3));
    in.get(reinterpret_cast<char&>(b4));
    int totalChars = ((int)b1 << 24) | ((int)b2 << 16) | ((int)b3 << 8) | (int)b4;
    
    std::unordered_map<char, int> freq;
    for (int i = 0; i < numChars; i++) {
        unsigned char ch;
        in.get(reinterpret_cast<char&>(ch));
        in.get(reinterpret_cast<char&>(b1));
        in.get(reinterpret_cast<char&>(b2));
        in.get(reinterpret_cast<char&>(b3));
        in.get(reinterpret_cast<char&>(b4));
        int f = ((int)b1 << 24) | ((int)b2 << 16) | ((int)b3 << 8) | (int)b4;
        freq[ch] = f;
    }

    // Build tree
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

    // Decompress
    std::ofstream out(outFile, std::ios::binary);
    auto node = root;
    unsigned char buffer;
    int charsWritten = 0;
    char ch;
    while (in.get(ch) && charsWritten < totalChars) {
        buffer = (unsigned char)ch;
        for (int i = 0; i < 8 && charsWritten < totalChars; i++) {
            int bit = (buffer >> (7 - i)) & 1;
            if (bit == 0) node = node->left;
            else node = node->right;
            if (node && node->ch != '\0') {
                out.put(node->ch);
                charsWritten++;
                node = root;
            }
        }
    }
    in.close();
    out.close();

    std::cout << "Decompression complete!" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: decompress.exe <compressed_file> <output_file>" << std::endl;
        return 1;
    }
    decompress(argv[1], argv[2]);
    return 0;
}
