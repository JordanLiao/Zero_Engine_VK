#include "IO.h"

#include <fstream>
#include <iostream>

bool IO::readSPIRvBinary(const std::string& filePath, std::vector<uint32_t>& data) {
    std::ifstream file(filePath, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening the file." << std::endl;
        return false;
    }

    uint32_t value;
    while (file.read(reinterpret_cast<char*>(&value), sizeof(uint32_t))) {
        data.push_back(value);
    }

    file.close();
    return true;
}

bool IO::readFile(const std::string& filePath, std::vector<char>& data){
    std::ifstream file(filePath, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening the file." << std::endl;
        return false;
    }

    char value;
    while (file.read(reinterpret_cast<char*>(&value), sizeof(char))) {
        data.push_back(value);
    }

    file.close();
    return true;
}
