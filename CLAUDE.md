# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**quran-video-maker-ffmpeg** is a high-performance C++/FFmpeg tool for generating Quran verse videos with synchronized Arabic text, translations, and recitations. The project combines audio processing, text rendering with HarfBuzz/FreeType, and FFmpeg video generation to create professionally styled Quran videos.

## Development Commands

### Building
```bash
# Standard build
mkdir build && cd build
cmake ..
cmake --build .
cd ..

# Clean build (recommended for major changes)
rm -rf build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
cd ..

# Release build with installation
cmake -S . -B build-test -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PWD/build-test/install
cmake --build build-test --target install
```

### Testing
```bash
# Run all unit tests
cd build
ctest --output-on-failure
cd ..

# Run executable directly
./build/qvm 1 1 7 --reciter 2 --translation 1

# Test installed binary (from any directory)
export QVM_PREFIX=$PWD/build-test/install
export PATH="$QVM_PREFIX/bin:$PATH"
cd /tmp
qvm 1 1 7
```

### Required Data Setup
Before building or testing, download and extract test data:
```bash
curl -L https://qvm-r2-storage.tawbah.app/data.tar -o data.tar
tar -xf data.tar
```

## Architecture Overview

### Core Pipeline
The video generation follows this flow:
1. **CLI Parsing** (`main.cpp`) → Parse arguments with cxxopts
2. **Config Loading** (`config_loader.h/cpp`) → Load `config.json` and merge CLI overrides
3. **Data Fetching** (`recitation_utils.h/cpp`, `cache_utils.h/cpp`) → Fetch verse data, audio, translations
4. **Audio Processing** (`audio/custom_audio_processor.h/cpp`) → Handle gapped/gapless audio, custom recitations
5. **Text Layout** (`text/text_layout.h/cpp`) → Measure and wrap Arabic/translation text with HarfBuzz
6. **Subtitle Generation** (`subtitle_builder.h/cpp`) → Generate ASS subtitles with animations
7. **Video Encoding** (`video_generator.h/cpp`) → Render final video with FFmpeg

### Key Components

#### Type System (`types.h`)
- `AppConfig`: Runtime configuration merged from config.json and CLI
- `CLIOptions`: Command-line arguments
- `VerseData`: Verse text, translation, audio URL, timing data
- `RecitationMode`: GAPPED (ayah-by-ayah) vs GAPLESS (continuous)
- `VideoSelectionConfig`: Dynamic background video configuration

#### Recitation Modes
- **GAPPED**: Individual verse audio files with pauses between verses (default)
- **GAPLESS**: Single continuous audio file for entire surah with timing data
- Custom recitations supported via `--custom-audio` + `--custom-timing` (VTT/SRT files)

#### Data Layer
- `quran_data.h`: Static mappings for reciters, translations, fonts, language codes
- Reciter IDs map to JSON files in `data/ayah-by-ayah/` (gapped) or `data/surah-by-surah/` (gapless)
- Translation IDs map to JSON files in `data/translations/<lang>/`
- Word-by-word Quranic data in `data/quran/qpc-hafs-word-by-word.json`

#### Text Rendering (`text/text_layout.h/cpp`)
- Uses HarfBuzz for Arabic text shaping (critical for Uthmanic script)
- Uses FreeType for font metrics and glyph measurements
- Handles RTL/LTR text, multi-line wrapping, growth animations
- **Engine class**: Main API is `layoutVerse()` which returns wrapped text + font sizes
- Respects `textHorizontalPadding`, `textVerticalPadding`, `arabicMaxWidthFraction`, `translationMaxWidthFraction`

#### Audio Processing (`audio/custom_audio_processor.h/cpp`)
- **CustomAudioProcessor::buildSplicePlan()**: Plans audio extraction/concatenation for custom recitations
- Handles Bismillah insertion (from surah 1) for surahs that begin with it
- Re-bases verse timings when trimming audio ranges (e.g., verses 50-70 from full surah)
- **TimingParser** (`timing_parser.h/cpp`): Parses VTT/SRT files into `TimingEntry` structs

#### Subtitle System (`subtitle_builder.h/cpp`)
- Generates ASS (Advanced SubStation Alpha) format subtitles
- Implements text growth animations (`enableTextGrowth`, `maxGrowthFactor`, `growthRateFactor`)
- Fade-in/fade-out effects controlled by `fadeDurationFactor`, `minFadeDuration`, `maxFadeDuration`
- Calls `TextLayout::Engine` to get properly shaped and wrapped text

#### Dynamic Backgrounds (`background_video_manager.h/cpp`, `video_selector.h/cpp`)
- **VideoSelector**: Selects themed videos based on `metadata/surah-themes.json`
- Themes map verse ranges to video categories (e.g., "19:1-15" → ["dua", "newlife"])
- **BackgroundVideoManager**: Downloads/caches videos from R2 or uses local directory
- Supports deterministic selection via `--seed` flag
- Videos concatenated on-the-fly during render (no pre-stitching)

#### R2 Integration (`r2_client.h/cpp`)
- AWS S3-compatible client for Cloudflare R2
- Supports public buckets (anonymous) and private buckets (credentials)
- Used for fetching background videos and audio files
- Credentials via config.json (`${R2_ENDPOINT}` env var substitution) or CLI flags

#### Interfaces (`interfaces/`)
- `IProcessExecutor`: Abstracts FFmpeg process execution for testing
- `IApiClient`: Abstracts HTTP requests for testing
- Mock implementations in `tests/` for unit testing (`MockProcessExecutor`, `MockApiClient`)

#### Metadata & Progress
- `metadata_writer.h/cpp`: Writes JSON sidecars with render metadata (CLI args, config snapshot, paths)
- `--progress` flag emits structured `PROGRESS {...}` JSON logs for machine parsing
- Stages: background, subtitles, encoding (with percent, ETA)

#### Localization (`localization_utils.h/cpp`)
- Maps translation IDs to language codes (`translationLanguages` in `quran_data.h`)
- Loads localized labels from `data/misc/surah.json`, `data/misc/numbers.json`
- Loads reciter/surah names from `data/reciter-names/<lang>.json`, `data/surah-names/<lang>.json`
- Fallback to English for missing translations

### File Organization
```
src/
├── main.cpp                    # Entry point, CLI parsing
├── types.h                     # Core data structures
├── quran_data.h                # Static data mappings (reciters, translations)
├── config_loader.h/cpp         # Config loading and merging
├── video_generator.h/cpp       # Main video rendering pipeline
├── subtitle_builder.h/cpp      # ASS subtitle generation
├── timing_parser.h/cpp         # VTT/SRT parser
├── recitation_utils.h/cpp      # Verse data fetching, audio downloading
├── cache_utils.h/cpp           # Caching layer (audio, translations)
├── localization_utils.h/cpp    # Localized labels and names
├── metadata_writer.h/cpp       # Render metadata generation
├── background_video_manager.h/cpp  # Video download and caching
├── video_selector.h/cpp        # Theme-based video selection
├── video_standardizer.h/cpp    # Video preprocessing (1280x720@30fps)
├── r2_client.h/cpp             # R2/S3 client
├── text/
│   └── text_layout.h/cpp       # HarfBuzz text shaping and wrapping
├── audio/
│   └── custom_audio_processor.h/cpp  # Audio slicing/concatenation
└── interfaces/
    ├── IProcessExecutor.h      # FFmpeg execution interface
    └── IApiClient.h            # HTTP client interface
```

## Configuration System

### config.json Structure
- **Video settings**: width, height, fps
- **Content**: reciterId, translationId, recitationMode
- **Fonts**: arabicFont, translationFont (family, file, size, color)
- **Animation**: enableTextGrowth, textGrowthThreshold, maxGrowthFactor, growthRateFactor
- **Timing**: introDuration, pauseAfterIntroDuration, introFadeOutMs
- **Layout**: textHorizontalPadding, textVerticalPadding, arabicMaxWidthFraction, translationMaxWidthFraction
- **Quality Profiles**: speed/balanced/max presets (crf, preset, pixelFormat, videoBitrate)
- **Video Selection**: R2 credentials, theme metadata path, seed

### CLI Override Behavior
CLI flags override config.json values. The `config_loader.cpp` merges them with config taking precedence for CLI-provided values.

### Quality Profiles
Three built-in profiles in `config.json`:
- `speed`: ultrafast preset, CRF 27, yuv420p (fast previews)
- `balanced`: fast preset, CRF 21, yuv420p, 4500k bitrate (default)
- `max`: slow preset, CRF 18, yuv420p10le, 8000k bitrate (archival quality)

Override via `--quality-profile`, `--preset`, `--crf`, `--pix-fmt`, `--video-bitrate`, etc.

## Data Files and Formats

### Quran Word-by-Word Data
- Path: `data/quran/qpc-hafs-word-by-word.json`
- Format: JSON keyed by verse (e.g., `"1:1"`) with `{"t": "text", "w": [...]}`
- Contains Arabic text in Uthmanic script with word boundaries

### Translation Files
- Path: `data/translations/<lang>/<filename>.json`
- Format: JSON keyed by verse (e.g., `"2:255"`) with `{"t": "translation text"}`
- Follow [QUL format](https://qul.tarteel.ai/resources/translation)

### Recitation Metadata (Gapped Mode)
- Path: `data/ayah-by-ayah/<reciter_name>/<filename>.json`
- Format: JSON keyed by verse with `{"audio": "url", "duration": 1.23}`

### Recitation Metadata (Gapless Mode)
- Path: `data/surah-by-surah/<reciter_name>/<surah_number>.json`
- Format: JSON with `{"audio": "url", "verses": {"1": {"timestampFrom": 0, "timestampTo": 1500, ...}}}`
- Currently disabled in built-in data; use `--custom-audio` + `--custom-timing` instead

### Timing Files (VTT/SRT)
- Custom recitations require timing files with verse timestamps
- Format: Standard WebVTT or SRT with cue text containing verse numbers
- Example VTT:
  ```
  WEBVTT

  1
  00:00:00.000 --> 00:00:03.500
  1. بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ
  ```

### Theme Metadata
- Path: `metadata/surah-themes.json`
- Format: `{"surah_id": {"verse_range": ["theme1", "theme2"]}}`
- Example: `{"19": {"1-15": ["dua", "newlife"], "16-33": ["maryam", "birth"]}}`

## Adding New Translations

Follow this checklist (from CONTRIBUTING.md):

1. **Download QUL Translation**: Place JSON in `data/translations/<lang-code>/`
2. **Register in `src/quran_data.h`**:
   - Add to `translationFiles` with new ID
   - Add to `translationLanguages` with language code
   - Add to `translationFontMappings` and `translationFontFamilies`
   - Add to `translationDirectionIsRtl` (true for RTL scripts)
3. **Add Font**: Place font file in `assets/fonts/` if needed
4. **Localized Labels**: Edit `data/misc/surah.json` (word for "Surah"), `data/misc/numbers.json` (digits 1-114)
5. **Transliterated Names**: Create `data/surah-names/<lang>.json` and `data/reciter-names/<lang>.json`
6. **Test**: `./build/qvm 1 1 7 --translation <new-id>`

## Testing Guidelines

### Unit Tests
- Located in `tests/unit_tests.cpp`
- Test functions: `testConfigLoader()`, `testTimingParser()`, `testTextLayoutEngine()`, etc.
- Tests use mock implementations (`MockApiClient`, `MockProcessExecutor`)
- Run with `ctest --output-on-failure` from build directory

### Integration Testing
Always test with edge cases:
- Short surahs (Al-Fatiha 1:1-7)
- Long surahs (Al-Baqarah 2:1-286)
- Surah 9 (no Bismillah)
- Different reciters and translations
- Custom audio with verse range trimming

### Dynamic Background Testing
```bash
# Test local directory
mkdir -p test-videos/prayer test-videos/guidance
qvm 1 1 7 --enable-dynamic-bg --local-video-dir ./test-videos

# Test R2 with seed
qvm 19 1 40 --enable-dynamic-bg --seed 42

# Test standardization
qvm --standardize-local ./test-videos
```

## Important Implementation Notes

### FFmpeg Integration
- All FFmpeg calls go through `IProcessExecutor` interface for testability
- Video encoding uses complex filtergraphs for subtitles, overlays, backgrounds
- Verbose logging via `FFREPORT=file=ffmpeg.log:level=32 ./build/qvm ...`

### Arabic Text Rendering
- **Must use HarfBuzz**: Arabic requires proper text shaping (ligatures, diacritics)
- Uthmanic script has specific font requirements (`UthmanicHafs_V22.ttf`)
- Never skip HarfBuzz for Arabic text measurement or rendering
- RTL handling is critical for proper layout

### Audio Timing
- Gapless audio requires careful timestamp normalization (`RecitationUtils::normalizeGaplessTimings()`)
- Custom audio slicing handles negative offsets, Bismillah insertion, and rebasing
- Timing precision is in milliseconds but durations are often stored as seconds

### Caching
- Audio files cached in `.cache/audio/`
- Translation data cached via `cache_utils.cpp`
- Background videos cached per theme
- Clear cache with `--clear-cache` flag
- Disable caching with `--no-cache` flag

### Package Installation
- Homebrew: `brew install ashaltu/tap/qvm`
- Scoop: `scoop install https://github.com/ashaltu/quran-video-maker-ffmpeg/releases/latest/download/scoop-qvm.json`
- Data and assets installed to platform-specific share directories (e.g., `/opt/homebrew/share/quran-video-maker/`)

## Performance Considerations

- Text layout uses parallel processing for measurements
- Audio concatenation optimized in gapless mode
- Hardware encoding supported via `--encoder hardware` (macOS VideoToolbox)
- Quality presets balance speed vs quality
- Benchmarks (M1 MacBook Pro, ultrafast):
  - Al-Fatiha (1:1-7, gapped): ~5s
  - Al-Mu'minun (23:1-118, gapless): ~2m
  - Al-Baqarah (2:1-286, gapless): ~22m

## Common Pitfalls

1. **Missing data.tar**: Always download and extract before building/testing
2. **Incorrect font paths**: Paths in config.json are relative to `assetFolderPath`
3. **HarfBuzz dependency**: Text measurement differs from basic glyph width—always use `TextLayout::Engine`
4. **Gapless built-in data disabled**: Use `--custom-audio` + `--custom-timing` for continuous recitations
5. **Translation fallback fonts**: Set `translationFallbackFontFamily` for ASCII/Latin characters in non-Latin scripts
6. **Path resolution**: Installed binaries look for assets in share directories, not repo root
7. **Timing file format**: VTT/SRT cues must include verse numbers in text for parser to extract verse keys

## Project Mission

This tool exists to make Quranic content accessible and engaging for everyone (Muslims, non-Muslims, Arabic readers, non-Arabic readers) by creating beautiful videos that facilitate sincere reflection and understanding of the Quran. When contributing:

- Prioritize accessibility and ease of use
- Maintain high quality standards (the Quran deserves our best effort)
- Avoid features that encourage mindless consumption
- Ensure all content is free from shirk (compromise of tawheed)
- Focus on educational value and facilitating learning
