#include "tool.h"


bool writeFile(const std::string& filename, const unsigned char* data, size_t size) {
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    outFile.write(reinterpret_cast<const char*>(data), size);
    outFile.close();
    return true;
}