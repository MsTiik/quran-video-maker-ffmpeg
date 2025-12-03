#pragma once
#include <string>

namespace VideoStandardizer {
    void standardizeDirectory(const std::string& path, bool isR2Bucket = false);
    std::string getCurrentTimestamp();
}