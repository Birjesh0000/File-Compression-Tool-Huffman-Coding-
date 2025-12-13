#include <iostream>
#include <fstream>
#include <queue>
#include <memory>
#include <map>
#include <string>

struct Node {
    unsigned char ch;
    int freq;
    std::shared_ptr<Node> left, right;
    Node(unsigned char c = 0, int f = 0) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

struct CompareNode {
    bool operator()(const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) const {
        if (a->freq != b->freq) return a->freq > b->freq;
        return a->ch > b->ch;
    }
};

void decompress(const std::string& inFile, const std::string& outFile) {
    std::cout << "Decompressing " << inFile << "..." << std::endl;

    std::ifstream in(inFile, std::ios::binary);
    if (!in) {
        std::cerr << "Error: Cannot open compressed file." << std::endl;
        return;
    }

    unsigned char numChars;
    in.get((char&)numChars);
    
    unsigned char b1, b2, b3, b4;
    in.get((char&)b1);
    in.get((char&)b2);
    in.get((char&)b3);
    in.get((char&)b4);
    int totalChars = ((int)b1 << 24) | ((int)b2 << 16) | ((int)b3 << 8) | (int)b4;
    
    std::map<unsigned char, int> freq;
    for (int i = 0; i < numChars; i++) {
        unsigned char ch;
        in.get((char&)ch);
        in.get((char&)b1);
        in.get((char&)b2);
        in.get((char&)b3);
        in.get((char&)b4);
        int f = ((int)b1 << 24) | ((int)b2 << 16) | ((int)b3 << 8) | (int)b4;
        freq[ch] = f;
    }

    std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, CompareNode> pq;
    for (const auto& [c, f] : freq) pq.push(std::make_shared<Node>(c, f));
    
    while (pq.size() > 1) {
        auto left = pq.top(); pq.pop();
        auto right = pq.top(); pq.pop();
        auto parent = std::make_shared<Node>(0, left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }
    auto root = pq.top();

    std::ofstream out(outFile, std::ios::binary);
    auto node = root;
    unsigned char buffer;
    int charsWritten = 0;
    char ch;
    
    while (in.get(ch) && charsWritten < totalChars) {
        buffer = (unsigned char)ch;
        for (int i = 0; i < 8 && charsWritten < totalChars; i++) {
            int bit = (buffer >> (7 - i)) & 1;
            if (bit == 0 && node->left) node = node->left;
            else if (bit == 1 && node->right) node = node->right;
            
            if (node->ch != 0) {
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
    std::ifstream test(argv[1]);
    if (!test) {
        std::cerr << "Error: Compressed file not found." << std::endl;
        return 1;
    }
    test.close();
    decompress(argv[1], argv[2]);
    std::cout << "File saved to: " << argv[2] << std::endl;
    return 0;
}
