#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include <random>
#include <nlohmann/json.hpp>

namespace VideoSelector {

class SeededRandom {
public:
    explicit SeededRandom(unsigned int seed);
    int nextInt(int min, int max);
    
    template<typename T>
    const T& choice(const std::vector<T>& items);

private:
    std::mt19937 gen;
};

struct SelectionState {
    std::map<std::string, std::set<std::string>> usedVideos;  // theme -> set of used video keys
    std::map<std::string, std::set<std::string>> exhaustedThemesPerRange;  // range key -> exhausted themes
};

// Represents a verse range with its themes and time allocation
struct VerseRangeSegment {
    int startVerse;
    int endVerse;
    std::vector<std::string> themes;
    double startTimeFraction;  // 0.0 to 1.0 - when this range starts
    double endTimeFraction;    // 0.0 to 1.0 - when this range ends
    std::string rangeKey;      // e.g., "19:10-15" for tracking
};

class Selector {
public:
    explicit Selector(const std::string& metadataPath, unsigned int seed = 99);
    
    // Get themes for a verse range (original method - returns all themes)
    std::vector<std::string> getThemesForVerses(int surah, int from, int to);
    
    // Get verse range segments with time allocations for the requested range
    std::vector<VerseRangeSegment> getVerseRangeSegments(int surah, int from, int to);
    
    // Get themes for a specific time position (0.0 to 1.0)
    const VerseRangeSegment* getRangeForTimePosition(
        const std::vector<VerseRangeSegment>& segments,
        double timeFraction);
    
    // Select a theme from available themes for a specific range
    std::string selectThemeForRange(
        const VerseRangeSegment& range,
        const std::map<std::string, std::vector<std::string>>& themeVideosCache,
        SelectionState& state);
    
    // Select a video from theme ensuring no repeats until exhausted
    std::string selectVideoFromTheme(const std::string& theme,
                                    const std::vector<std::string>& availableVideos,
                                    SelectionState& state);

    // Legacy method for backward compatibility
    std::string selectTheme(const std::vector<std::string>& themes, 
                           const std::string& verseRange,
                           SelectionState& state);

private:
    nlohmann::json metadata;
    SeededRandom random;
    
    std::vector<int> parseVerseRange(const std::string& rangeStr);
    std::vector<std::string> findRangeForVerse(int surah, int verse);
    std::pair<int, int> findRangeBoundsForVerse(int surah, int verse);
};

} // namespace VideoSelector