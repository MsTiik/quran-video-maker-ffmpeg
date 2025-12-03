#include "background_video_manager.h"
#include "r2_client.h"
#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>

extern "C" {
#include <libavformat/avformat.h>
}

namespace fs = std::filesystem;

namespace BackgroundVideo {

Manager::Manager(const AppConfig& config, const CLIOptions& options)
    : config_(config), options_(options) {
    auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    tempDir_ = fs::temp_directory_path() / ("qvm_bg_" + std::to_string(timestamp));
    fs::create_directories(tempDir_);
}

double Manager::getVideoDuration(const std::string& path) {
    AVFormatContext* formatContext = nullptr;
    if (avformat_open_input(&formatContext, path.c_str(), nullptr, nullptr) != 0) {
        return 0.0;
    }
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        avformat_close_input(&formatContext);
        return 0.0;
    }
    double duration = static_cast<double>(formatContext->duration) / AV_TIME_BASE;
    avformat_close_input(&formatContext);
    return duration;
}

std::vector<VideoSegment> Manager::collectVideoSegments(double targetDuration) {
    std::vector<VideoSegment> segments;
    double totalDuration = 0.0;
    
    std::cout << "  Target duration: " << targetDuration << " seconds" << std::endl;
    std::cout << "  Collecting video segments..." << std::endl;
    
    // Initialize selector
    VideoSelector::Selector selector(
        config_.videoSelection.themeMetadataPath,
        config_.videoSelection.seed
    );
    
    // Get all available themes for the verse range
    auto availableThemes = selector.getThemesForVerses(
        options_.surah, 
        options_.from, 
        options_.to
    );
    
    if (availableThemes.empty()) {
        throw std::runtime_error("No themes available for the specified verse range");
    }
    
    std::cout << "  Available themes: ";
    for (size_t i = 0; i < availableThemes.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << availableThemes[i];
    }
    std::cout << std::endl;
    
    // Initialize R2 client
    R2::R2Config r2Config{
        config_.videoSelection.r2Endpoint,
        config_.videoSelection.r2AccessKey,
        config_.videoSelection.r2SecretKey,
        config_.videoSelection.r2Bucket,
        config_.videoSelection.usePublicBucket
    };
    R2::Client r2Client(r2Config);
    
    // Keep track of available videos per theme
    std::map<std::string, std::vector<std::string>> themeVideosCache;
    
    // Collect segments until we meet or exceed the target duration
    int segmentCount = 0;
    std::string verseRange = std::to_string(options_.surah) + ":" + 
                             std::to_string(options_.from) + "-" + 
                             std::to_string(options_.to);
    
    while (totalDuration < targetDuration) {
        segmentCount++;
        
        // Select a theme (ensuring no repeats until exhausted)
        std::string selectedTheme = selector.selectTheme(
            availableThemes, 
            verseRange, 
            selectionState_
        );
        
        std::cout << "  Segment " << segmentCount << " - theme: " << selectedTheme;
        
        // Get available videos for this theme (cached)
        if (themeVideosCache.find(selectedTheme) == themeVideosCache.end()) {
            themeVideosCache[selectedTheme] = r2Client.listVideosInTheme(selectedTheme);
            if (themeVideosCache[selectedTheme].empty()) {
                std::cerr << " (no videos found, skipping)" << std::endl;
                // Mark this theme as exhausted so we don't try it again
                selectionState_.exhaustedThemes[verseRange].push_back(selectedTheme);
                continue;
            }
        }
        
        const auto& availableVideos = themeVideosCache[selectedTheme];
        
        // Select a video from this theme (ensuring no repeats until exhausted)
        std::string selectedVideo = selector.selectVideoFromTheme(
            selectedTheme,
            availableVideos,
            selectionState_
        );
        
        std::cout << ", video: " << fs::path(selectedVideo).filename();
        
        // Download the video
        fs::path localPath = tempDir_ / (std::to_string(segmentCount) + "_" + fs::path(selectedVideo).filename().string());
        std::string downloadedPath = r2Client.downloadVideo(selectedVideo, localPath);
        tempFiles_.push_back(localPath);
        
        // Get video duration
        double videoDuration = getVideoDuration(downloadedPath);
        if (videoDuration <= 0) {
            std::cerr << " (invalid duration, skipping)" << std::endl;
            continue;
        }
        
        std::cout << ", duration: " << videoDuration << "s" << std::endl;
        
        // Add to segments
        VideoSegment segment;
        segment.path = downloadedPath;
        segment.theme = selectedTheme;
        segment.duration = videoDuration;
        segment.isLocal = true;
        segments.push_back(segment);
        
        totalDuration += videoDuration;
        
        // Check if all themes and videos are exhausted
        bool allExhausted = true;
        for (const auto& theme : availableThemes) {
            if (themeVideosCache.find(theme) != themeVideosCache.end()) {
                const auto& videos = themeVideosCache[theme];
                if (!videos.empty() && selectionState_.usedVideos[theme].size() < videos.size()) {
                    allExhausted = false;
                    break;
                }
            }
        }
        
        if (allExhausted && totalDuration < targetDuration) {
            std::cout << "  All unique videos exhausted, resetting selection state..." << std::endl;
            selectionState_.usedVideos.clear();
            selectionState_.exhaustedThemes[verseRange].clear();
        }
        
        // Safety limit to prevent infinite loops
        if (segmentCount > 100) {
            std::cerr << "  Warning: Reached segment limit, stopping collection" << std::endl;
            break;
        }
    }
    
    std::cout << "  Collected " << segments.size() << " segments, total duration: " 
              << totalDuration << " seconds" << std::endl;
    
    return segments;
}

std::string Manager::stitchVideos(const std::vector<VideoSegment>& segments) {
    if (segments.empty()) {
        throw std::runtime_error("No video segments to stitch");
    }
    
    // If only one segment, return it directly
    if (segments.size() == 1) {
        std::cout << "  Single segment, using directly" << std::endl;
        return segments[0].path;
    }
    
    std::cout << "  Stitching " << segments.size() << " video segments..." << std::endl;
    
    // Create concat demuxer file
    fs::path concatFile = tempDir_ / "concat.txt";
    std::ofstream concat(concatFile);
    if (!concat.is_open()) {
        throw std::runtime_error("Failed to create concat file");
    }
    
    for (const auto& segment : segments) {
        concat << "file '" << fs::absolute(segment.path).string() << "'\n";
    }
    concat.close();
    
    // Output path for stitched video
    fs::path outputPath = tempDir_ / "background_stitched.mp4";
    tempFiles_.push_back(outputPath);
    
    // Build ffmpeg command
    std::ostringstream cmd;
    cmd << "ffmpeg -y -f concat -safe 0 -i \"" << concatFile.string() << "\" ";
    cmd << "-c copy ";  // Copy codecs for speed
    cmd << "-movflags +faststart ";
    cmd << "\"" << outputPath.string() << "\"";
    
    std::cout << "  Running: " << cmd.str() << std::endl;
    
    // Execute ffmpeg
    int result = std::system(cmd.str().c_str());
    if (result != 0) {
        throw std::runtime_error("Failed to stitch videos with ffmpeg");
    }
    
    // Verify the output exists
    if (!fs::exists(outputPath)) {
        throw std::runtime_error("Stitched video file not created");
    }
    
    double stitchedDuration = getVideoDuration(outputPath.string());
    std::cout << "  Stitched video created, duration: " << stitchedDuration << " seconds" << std::endl;
    
    return outputPath.string();
}

std::string Manager::prepareBackgroundVideo(double totalDurationSeconds) {
    // If dynamic backgrounds disabled, use default
    if (!config_.videoSelection.enableDynamicBackgrounds) {
        return config_.assetBgVideo;
    }

    try {
        std::cout << "Selecting dynamic background videos..." << std::endl;
        
        // Collect video segments to meet the target duration
        std::vector<VideoSegment> segments = collectVideoSegments(totalDurationSeconds);
        
        if (segments.empty()) {
            std::cerr << "Warning: No video segments collected, using default background" << std::endl;
            return config_.assetBgVideo;
        }
        
        // Stitch videos together if needed
        std::string finalVideo = stitchVideos(segments);
        
        // Check if we need to note about looping
        double finalDuration = getVideoDuration(finalVideo);
        if (finalDuration > 0 && finalDuration < totalDurationSeconds) {
            std::cout << "  Note: Stitched background duration (" << finalDuration 
                     << "s) is shorter than total duration (" << totalDurationSeconds 
                     << "s), will loop automatically" << std::endl;
        }
        
        std::cout << "  Background video ready: " << finalVideo << std::endl;
        return finalVideo;
        
    } catch (const std::exception& e) {
        std::cerr << "Warning: Dynamic background selection failed: " << e.what() 
                 << ", using default background" << std::endl;
        return config_.assetBgVideo;
    }
}

void Manager::cleanup() {
    for (const auto& file : tempFiles_) {
        std::error_code ec;
        fs::remove(file, ec);
    }
    if (fs::exists(tempDir_)) {
        std::error_code ec;
        fs::remove_all(tempDir_, ec);
    }
}

} // namespace BackgroundVideo