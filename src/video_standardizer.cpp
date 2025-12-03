#include "video_standardizer.h"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <nlohmann/json.hpp>

extern "C" {
#include <libavformat/avformat.h>
}

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace VideoStandardizer {

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &time_t);
#else
    gmtime_r(&time_t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

void standardizeDirectory(const std::string& path, bool isR2Bucket) {
    if (isR2Bucket) {
        std::cout << "R2 bucket standardization not yet implemented" << std::endl;
        std::cout << "Please download the bucket locally first, standardize, then re-upload" << std::endl;
        return;
    }
    
    if (!fs::exists(path)) {
        throw std::runtime_error("Directory does not exist: " + path);
    }
    
    std::cout << "Standardizing videos in: " << path << std::endl;
    
    json metadata;
    metadata["standardizedAt"] = getCurrentTimestamp();
    metadata["videos"] = json::array();
    
    int totalVideos = 0;
    double totalDuration = 0.0;
    
    // Process each theme directory
    for (const auto& themeEntry : fs::directory_iterator(path)) {
        if (!themeEntry.is_directory()) continue;
        
        std::string theme = themeEntry.path().filename().string();
        std::cout << "\nProcessing theme: " << theme << std::endl;
        
        for (const auto& videoEntry : fs::directory_iterator(themeEntry)) {
            if (!videoEntry.is_regular_file()) continue;
            
            std::string ext = videoEntry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            
            if (ext != ".mp4" && ext != ".mov" && ext != ".avi" && 
                ext != ".mkv" && ext != ".webm") continue;
            
            // Skip already standardized files
            auto stem = videoEntry.path().stem().string();
            // (s.size() >= 4 && s.compare(s.size() - 4, 4, "_std") == 0)
            if (stem.size() >= 4 && stem.compare(stem.size() - 4, 4, "_std") == 0){
                std::cout << "  Already standardized: " << videoEntry.path().filename() << std::endl;
                continue;
            }
            
            // Standardize the video
            fs::path outputPath = videoEntry.path().parent_path() / 
                                  (videoEntry.path().stem().string() + "_std.mp4");
            
            std::ostringstream cmd;
            cmd << "ffmpeg -y -i \"" << videoEntry.path().string() << "\" "
                << "-c:v libx264 -preset fast -crf 23 "
                << "-s 1280x720 -r 30 "
                << "-pix_fmt yuv420p "
                << "-an "  // Remove audio
                << "-movflags +faststart "
                << "\"" << outputPath.string() << "\" 2>/dev/null";
            
            std::cout << "  Standardizing: " << videoEntry.path().filename() << " -> " 
                      << outputPath.filename() << std::endl;
            
            int result = std::system(cmd.str().c_str());
            if (result == 0 && fs::exists(outputPath)) {
                // Get duration
                AVFormatContext* ctx = nullptr;
                double duration = 0.0;
                if (avformat_open_input(&ctx, outputPath.string().c_str(), nullptr, nullptr) == 0) {
                    if (avformat_find_stream_info(ctx, nullptr) >= 0) {
                        duration = static_cast<double>(ctx->duration) / AV_TIME_BASE;
                    }
                    avformat_close_input(&ctx);
                }
                
                // Remove original
                fs::remove(videoEntry.path());
                
                // Add to metadata
                json videoInfo;
                videoInfo["theme"] = theme;
                videoInfo["filename"] = outputPath.filename().string();
                videoInfo["duration"] = duration;
                metadata["videos"].push_back(videoInfo);
                
                totalVideos++;
                totalDuration += duration;
            } else {
                std::cerr << "  Failed to standardize: " << videoEntry.path().filename() << std::endl;
            }
        }
    }
    
    metadata["totalVideos"] = totalVideos;
    metadata["totalDuration"] = totalDuration;
    
    // Save metadata
    fs::path metadataPath = fs::path(path) / "metadata.json";
    std::ofstream metaFile(metadataPath);
    metaFile << metadata.dump(2);
    
    std::cout << "\n✅ Standardization complete!" << std::endl;
    std::cout << "Total videos: " << totalVideos << std::endl;
    std::cout << "Total duration: " << totalDuration << " seconds" << std::endl;
    std::cout << "Metadata saved to: " << metadataPath << std::endl;
}

} // namespace VideoStandardizer