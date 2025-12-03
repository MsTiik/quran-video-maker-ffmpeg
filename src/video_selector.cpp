#include "video_selector.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

namespace VideoSelector {

SeededRandom::SeededRandom(unsigned int seed) : gen(seed) {}

int SeededRandom::nextInt(int min, int max) {
    if (min >= max) return min;
    std::uniform_int_distribution<> dis(min, max - 1);
    return dis(gen);
}

template<typename T>
const T& SeededRandom::choice(const std::vector<T>& items) {
    if (items.empty()) {
        throw std::runtime_error("Cannot choose from empty vector");
    }
    return items[nextInt(0, items.size())];
}

// Explicit template instantiation
template const std::string& SeededRandom::choice<std::string>(const std::vector<std::string>&);

Selector::Selector(const std::string& metadataPath, unsigned int seed)
    : random(seed) {
    std::ifstream file(metadataPath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open theme metadata: " + metadataPath);
    }
    file >> metadata;
}

std::vector<int> Selector::parseVerseRange(const std::string& rangeStr) {
    std::vector<int> verses;
    std::istringstream iss(rangeStr);
    std::string part;
    
    while (std::getline(iss, part, ',')) {
        size_t dashPos = part.find('-');
        if (dashPos != std::string::npos) {
            int start = std::stoi(part.substr(0, dashPos));
            int end = std::stoi(part.substr(dashPos + 1));
            for (int i = start; i <= end; ++i) {
                verses.push_back(i);
            }
        } else {
            verses.push_back(std::stoi(part));
        }
    }
    
    std::sort(verses.begin(), verses.end());
    verses.erase(std::unique(verses.begin(), verses.end()), verses.end());
    return verses;
}

std::pair<int, int> Selector::findRangeBoundsForVerse(int surah, int verse) {
    std::string surahKey = std::to_string(surah);
    if (!metadata.contains(surahKey)) {
        return {-1, -1};
    }
    
    const auto& surahData = metadata[surahKey];
    for (auto it = surahData.begin(); it != surahData.end(); ++it) {
        const std::string& range = it.key();
        size_t dashPos = range.find('-');
        if (dashPos == std::string::npos) continue;
        
        int start = std::stoi(range.substr(0, dashPos));
        int end = std::stoi(range.substr(dashPos + 1));
        
        if (verse >= start && verse <= end) {
            return {start, end};
        }
    }
    
    return {-1, -1};
}

std::vector<std::string> Selector::findRangeForVerse(int surah, int verse) {
    std::string surahKey = std::to_string(surah);
    if (!metadata.contains(surahKey)) {
        return {};
    }
    
    const auto& surahData = metadata[surahKey];
    for (auto it = surahData.begin(); it != surahData.end(); ++it) {
        const std::string& range = it.key();
        size_t dashPos = range.find('-');
        if (dashPos == std::string::npos) continue;
        
        int start = std::stoi(range.substr(0, dashPos));
        int end = std::stoi(range.substr(dashPos + 1));
        
        if (verse >= start && verse <= end) {
            return it.value().get<std::vector<std::string>>();
        }
    }
    
    return {};
}

std::vector<std::string> Selector::getThemesForVerses(int surah, int from, int to) {
    std::set<std::string> allThemes;
    
    for (int verse = from; verse <= to; ++verse) {
        auto themes = findRangeForVerse(surah, verse);
        allThemes.insert(themes.begin(), themes.end());
    }
    
    return std::vector<std::string>(allThemes.begin(), allThemes.end());
}

std::vector<VerseRangeSegment> Selector::getVerseRangeSegments(int surah, int from, int to) {
    std::vector<VerseRangeSegment> segments;
    
    // Find all unique verse ranges that overlap with our requested range
    std::map<std::string, VerseRangeSegment> rangeMap;
    
    for (int verse = from; verse <= to; ++verse) {
        auto bounds = findRangeBoundsForVerse(surah, verse);
        if (bounds.first < 0) continue;
        
        std::string rangeKey = std::to_string(surah) + ":" + 
                               std::to_string(bounds.first) + "-" + 
                               std::to_string(bounds.second);
        
        if (rangeMap.find(rangeKey) == rangeMap.end()) {
            VerseRangeSegment segment;
            segment.rangeKey = rangeKey;
            // Clamp to our requested range
            segment.startVerse = std::max(bounds.first, from);
            segment.endVerse = std::min(bounds.second, to);
            segment.themes = findRangeForVerse(surah, verse);
            rangeMap[rangeKey] = segment;
        } else {
            // Update the end verse if needed (in case of overlapping discovery)
            rangeMap[rangeKey].endVerse = std::max(rangeMap[rangeKey].endVerse, 
                                                    std::min(bounds.second, to));
        }
    }
    
    // Convert to vector and sort by start verse
    for (auto& [key, segment] : rangeMap) {
        segments.push_back(segment);
    }
    
    std::sort(segments.begin(), segments.end(), 
              [](const VerseRangeSegment& a, const VerseRangeSegment& b) {
                  return a.startVerse < b.startVerse;
              });
    
    // Calculate time fractions based on verse counts
    int totalVerses = to - from + 1;
    double currentFraction = 0.0;
    
    for (auto& segment : segments) {
        int verseCount = segment.endVerse - segment.startVerse + 1;
        double fraction = static_cast<double>(verseCount) / totalVerses;
        
        segment.startTimeFraction = currentFraction;
        segment.endTimeFraction = currentFraction + fraction;
        currentFraction += fraction;
    }
    
    // Ensure last segment ends at exactly 1.0
    if (!segments.empty()) {
        segments.back().endTimeFraction = 1.0;
    }
    
    return segments;
}

const VerseRangeSegment* Selector::getRangeForTimePosition(
    const std::vector<VerseRangeSegment>& segments,
    double timeFraction) {
    
    for (const auto& segment : segments) {
        if (timeFraction >= segment.startTimeFraction && 
            timeFraction < segment.endTimeFraction) {
            return &segment;
        }
    }
    
    // If at exactly 1.0 or beyond, return the last segment
    if (!segments.empty() && timeFraction >= segments.back().startTimeFraction) {
        return &segments.back();
    }
    
    return nullptr;
}

std::string Selector::selectThemeForRange(
    const VerseRangeSegment& range,
    const std::map<std::string, std::vector<std::string>>& themeVideosCache,
    SelectionState& state) {
    
    if (range.themes.empty()) {
        throw std::runtime_error("No themes available for range: " + range.rangeKey);
    }
    
    // Get exhausted themes for this specific range
    auto& exhaustedForRange = state.exhaustedThemesPerRange[range.rangeKey];
    
    // Filter to themes that have videos and aren't exhausted
    std::vector<std::string> available;
    for (const auto& theme : range.themes) {
        auto cacheIt = themeVideosCache.find(theme);
        if (cacheIt == themeVideosCache.end() || cacheIt->second.empty()) {
            continue;  // No videos for this theme
        }
        
        if (exhaustedForRange.find(theme) == exhaustedForRange.end()) {
            available.push_back(theme);
        }
    }
    
    // If all themes exhausted for this range, reset
    if (available.empty()) {
        std::cout << "    All themes exhausted for range " << range.rangeKey << ", resetting..." << std::endl;
        exhaustedForRange.clear();
        
        // Rebuild available list
        for (const auto& theme : range.themes) {
            auto cacheIt = themeVideosCache.find(theme);
            if (cacheIt != themeVideosCache.end() && !cacheIt->second.empty()) {
                available.push_back(theme);
            }
        }
        
        if (available.empty()) {
            throw std::runtime_error("No themes with videos available for range: " + range.rangeKey);
        }
    }
    
    // Select randomly from available themes
    return random.choice(available);
}

std::string Selector::selectTheme(const std::vector<std::string>& themes,
                                 const std::string& verseRange,
                                 SelectionState& state) {
    if (themes.empty()) {
        throw std::runtime_error("No themes available for selection");
    }
    
    // Legacy implementation - kept for backward compatibility
    auto& exhausted = state.exhaustedThemesPerRange[verseRange];
    
    std::vector<std::string> available;
    for (const auto& theme : themes) {
        if (exhausted.find(theme) == exhausted.end()) {
            available.push_back(theme);
        }
    }
    
    if (available.empty()) {
        exhausted.clear();
        available = themes;
    }
    
    return random.choice(available);
}

std::string Selector::selectVideoFromTheme(const std::string& theme,
                                          const std::vector<std::string>& availableVideos,
                                          SelectionState& state) {
    if (availableVideos.empty()) {
        throw std::runtime_error("No videos available in theme: " + theme);
    }
    
    auto& used = state.usedVideos[theme];
    std::vector<std::string> unused;
    
    // Find unused videos
    for (const auto& video : availableVideos) {
        if (used.find(video) == used.end()) {
            unused.push_back(video);
        }
    }
    
    // If all videos used, reset for this theme
    if (unused.empty()) {
        std::cout << "    All videos in theme '" << theme << "' used, resetting..." << std::endl;
        used.clear();
        unused = availableVideos;
    }
    
    // Select randomly from unused videos
    std::string selected = random.choice(unused);
    used.insert(selected);
    
    return selected;
}

} // namespace VideoSelector