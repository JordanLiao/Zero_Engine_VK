#ifndef _IO_H_
#define _IO_H_

#include <vector>
#include <string>

namespace IO {
    bool readSPIRvBinary(const std::string filePath, std::vector<uint32_t>& data);
}

#endif
