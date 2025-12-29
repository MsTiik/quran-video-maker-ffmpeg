# Quran Video Maker - Quick Start Guide

*A simple guide for creating beautiful Quran videos for Instagram*

---

## 1. Getting Started - Your First Video in 5 Minutes

### Opening Terminal

1. Press `Cmd + Space` (opens Spotlight)
2. Type `Terminal` and press Enter
3. A black window will appear - this is where you'll type commands

### Creating Your First Video

Let's create a simple video of Surah Al-Fatiha (the opening chapter):

```bash
cd ~/Documents/Quran_Video_test/quran-video-maker-ffmpeg
./build/qvm 1 1 7 --output ~/Desktop/my-first-quran-video.mp4 --width 720 --height 1280
```

**What this does:**
- `1 1 7` means: Surah 1, verses 1 through 7 (Al-Fatiha - The Opening)
- `--output ~/Desktop/my-first-quran-video.mp4` saves the video to your Desktop
- `--width 720 --height 1280` makes it 9:16 vertical format, perfect for Instagram Reels

Press Enter and wait 10-30 seconds. Your video will appear on the Desktop!

### Understanding the Basic Command Pattern

Every command follows this pattern:
```bash
./build/qvm [SURAH] [START_VERSE] [END_VERSE] [OPTIONS]
```

**Examples:**
- Surah 112 (Al-Ikhlas): `./build/qvm 112 1 4`
- Surah 67, verses 1-5: `./build/qvm 67 1 5`
- Surah 36 (Ya-Sin), verses 1-12: `./build/qvm 36 1 12`

---

## 2. Command Examples - Different Videos, Different Results

Here are real examples showing how different options create different videos:

### Basic Video (No Background Videos)
```bash
cd ~/Documents/Quran_Video_test/quran-video-maker-ffmpeg
./build/qvm 67 1 5 --output ~/Desktop/surah67-basic.mp4 --width 720 --height 1280
```
**Result:** Simple video with just Arabic text, English translation, and a static starry background.

---

### With Dynamic Themed Backgrounds
```bash
./build/qvm 67 1 5 --output ~/Desktop/surah67-dynamic.mp4 --width 720 --height 1280 --enable-dynamic-bg --local-video-dir ~/Desktop/bg_videos
```
**Result:** Same verses, but now the tool automatically picks videos from your theme folders based on the surah's meaning. For Surah Al-Mulk (67), it uses videos from `sky`, `stars`, `creation`, and `night` themes.

**Why this matters:** Each surah has themes assigned to it (see Section 4), making the background relevant to what's being recited.

---

### Different Reciter
```bash
./build/qvm 36 1 12 --reciter 19 --output ~/Desktop/yaseen-mishari.mp4 --width 720 --height 1280 --enable-dynamic-bg --local-video-dir ~/Desktop/bg_videos
```
**Result:** Surah Ya-Sin (36) with Mishari Rashid al-Afasy's recitation instead of the default. Each reciter has a unique style and voice.

---

### Getting Different Background Videos (The Variety Solution!)
```bash
./build/qvm 93 1 11 --seed 1 --output ~/Desktop/duha-v1.mp4 --width 720 --height 1280 --enable-dynamic-bg --local-video-dir ~/Desktop/bg_videos

./build/qvm 93 1 11 --seed 2 --output ~/Desktop/duha-v2.mp4 --width 720 --height 1280 --enable-dynamic-bg --local-video-dir ~/Desktop/bg_videos

./build/qvm 93 1 11 --seed 3 --output ~/Desktop/duha-v3.mp4 --width 720 --height 1280 --enable-dynamic-bg --local-video-dir ~/Desktop/bg_videos
```
**Result:** Three different videos of the same verses! The `--seed` number controls which videos get picked from your theme folders.

**This is the key to variety:** Just change the seed number (1, 2, 3, 42, 100, etc.) to get different combinations of background videos. This way you can create multiple versions and pick your favorite!

---

### Fast Preview (Lower Quality, Faster Rendering)
```bash
./build/qvm 55 1 10 --quality-profile speed --output ~/Desktop/rahman-preview.mp4 --width 720 --height 1280 --enable-dynamic-bg --local-video-dir ~/Desktop/bg_videos
```
**Result:** Renders in about half the time, but slightly lower quality. Great for testing before making the final version.

---

### High Quality (Best for Final Posts)
```bash
./build/qvm 55 1 10 --quality-profile max --output ~/Desktop/rahman-final.mp4 --width 720 --height 1280 --enable-dynamic-bg --local-video-dir ~/Desktop/bg_videos
```
**Result:** Takes longer to render but produces the highest quality. Use this for your final Instagram posts.

---

## 3. Customization Options

### Choosing a Reciter

Add `--reciter [NUMBER]` to any command. Here are the most popular reciters:

| ID | Reciter Name | Style |
|----|--------------|-------|
| 2  | Abdur Rahman as-Sudais | Clear, melodious (Imam of Masjid al-Haram) |
| 19 | Mishari Rashid al-Afasy | Popular, emotional |
| 21 | Abdul Basit Abdul Samad | Classic, legendary voice |
| 25 | Maher Al-Muaiqly | Beautiful, emotional |
| 27 | Abu Bakr al-Shatri | Clear, fast-paced |
| 28 | Saad al-Ghamdi | Smooth, calming |

**Example:**
```bash
./build/qvm 18 1 10 --reciter 21 --output ~/Desktop/kahf-basit.mp4 --width 720 --height 1280
```

*See Section 5 for the complete list of all 27 available reciters.*

---

### Quality Profiles

Control render speed and video quality:

| Profile | Command | When to Use |
|---------|---------|-------------|
| **Speed** | `--quality-profile speed` | Quick previews, testing ideas |
| **Balanced** | (default, no flag needed) | Good quality, reasonable render time |
| **Max** | `--quality-profile max` | Final versions for posting |

**Example:**
```bash
./build/qvm 112 1 4 --quality-profile max --output ~/Desktop/ikhlas-final.mp4 --width 720 --height 1280
```

---

### Getting More Variety in Background Videos

**The Problem:** Your videos look too similar because the same clips keep showing up.

**The Solution:** Use the `--seed` flag with different numbers!

The tool randomly picks videos from your theme folders. The `--seed` number controls this randomness. Same seed = same videos. Different seed = different videos!

**Quick tip for variety:**
```bash
# Try 5 different versions and pick your favorite
./build/qvm 36 1 12 --seed 10 --output ~/Desktop/yaseen-v1.mp4 --width 720 --height 1280 --enable-dynamic-bg --local-video-dir ~/Desktop/bg_videos
./build/qvm 36 1 12 --seed 20 --output ~/Desktop/yaseen-v2.mp4 --width 720 --height 1280 --enable-dynamic-bg --local-video-dir ~/Desktop/bg_videos
./build/qvm 36 1 12 --seed 30 --output ~/Desktop/yaseen-v3.mp4 --width 720 --height 1280 --enable-dynamic-bg --local-video-dir ~/Desktop/bg_videos
```

**About video length:** If your background videos are very long (over 2-3 minutes), the tool will use fewer clips, giving less variety. Shorter clips (10-30 seconds) = more variety!

---

### Naming Your Videos

Use `--output` to choose where to save and what to name your video:

```bash
# Save to Desktop with a specific name
--output ~/Desktop/fajr-reminder.mp4

# Save to a folder on Desktop
--output ~/Desktop/Instagram-Posts/monday-post.mp4
```

---

### Turning Off Dynamic Backgrounds

If you just want the simple starry background:

```bash
./build/qvm 1 1 7 --output ~/Desktop/simple.mp4 --width 720 --height 1280
```

Just don't include `--enable-dynamic-bg` or `--local-video-dir` flags!

---

## 4. Adding Video Themes to Surahs

The tool automatically picks background videos based on the meaning of each surah. You control this by editing a simple file.

### Understanding How Themes Work

Your background videos are organized in folders by theme:
```
~/Desktop/bg_videos/
├── prayer/
├── sky/
├── ocean/
├── sunrise/
├── peace/
└── ...etc
```

The file `metadata/surah-themes.json` tells the tool: "For Surah X, verses Y-Z, use videos from these theme folders."

### Example: Current Setup for Surah Ya-Sin (36)

```json
"36": {
  "_name": "Ya-Sin",
  "1-27": ["light", "wisdom", "book"],
  "28-50": ["creation", "sky", "stars"],
  "51-83": ["gardens", "peace", "light"]
}
```

This means:
- Verses 1-27: Use videos from `light/`, `wisdom/`, and `book/` folders
- Verses 28-50: Use videos from `creation/`, `sky/`, and `stars/` folders
- Verses 51-83: Use videos from `gardens/`, `peace/`, and `light/` folders

### How to Add Themes for a New Surah

1. **Open the file in a text editor:**
   - Navigate to: `~/Documents/Quran_Video_test/quran-video-maker-ffmpeg/metadata/`
   - Right-click `surah-themes.json` → Open With → TextEdit

2. **Add your surah** following this pattern:

```json
"18": {
  "_name": "Al-Kahf - The Cave",
  "1-31": ["cave", "journey", "reflection"],
  "32-59": ["gardens", "nature", "wisdom"],
  "60-110": ["ocean", "journey", "wisdom"]
}
```

3. **Important formatting rules:**
   - Put a comma after the previous surah's closing `}`
   - Use `"` quotes around everything (surah numbers, verse ranges, themes)
   - Theme names must match your folder names exactly (case-sensitive!)
   - Verse ranges use `-` (e.g., `"1-15"`, not `"1 to 15"`)

### Available Theme Folders

Based on your current setup:

| Theme Name | Good For |
|------------|----------|
| `prayer` | Verses about worship, dua |
| `sky` | Creation, heavens, power of Allah |
| `stars` | Night verses, celestial themes |
| `ocean` | Stories of ships, mercy, vastness |
| `sunrise` / `sunset` | Morning/evening verses, hope |
| `peace` | Tranquility, reassurance |
| `light` | Guidance, clarity, truth |
| `nature` | Creation, reflection |
| `mountains` | Power, majesty |
| `journey` | Stories, prophets |
| `wisdom` | Teaching verses |
| `reflection` | Contemplation |
| `gardens` | Paradise, reward |
| `clouds` / `rain` | Mercy, provision |
| `storm` | Power, warning |
| `night` | Serenity, night prayers |

### Example: Adding Surah Al-Baqarah

```json
"2": {
  "_name": "Al-Baqarah - The Cow",
  "1-20": ["light", "book", "guidance"],
  "21-74": ["creation", "sky", "wisdom"],
  "75-141": ["journey", "reflection"],
  "142-242": ["prayer", "peace", "guidance"],
  "243-286": ["wisdom", "light", "peace"]
}
```

### Quick Tips

- **Use 2-4 themes per verse range** for good variety
- **Match themes to the content:** Read or think about what the verses discuss
- **Test your changes:** After editing, run a video to make sure it works
- **Keep backups:** Copy the file before making big changes

### Common Mistakes to Avoid

❌ Missing comma between surahs
❌ Themes that don't match folder names (check spelling!)
❌ Using single quotes `'` instead of double quotes `"`
❌ Forgetting the verse range format `"1-15"` (needs quotes!)

---

## 5. Quick Reference

### Complete Reciter List

| ID | Reciter Name |
|----|--------------|
| 2  | Abdur Rahman as-Sudais |
| 13 | Mahmoud Khalil Al-Husary |
| 14 | Hani ar-Rifai |
| 15 | Saud Al-Shuraim |
| 16 | Yasser Al-Dosari |
| 17 | Muhammad Siddiq Al-Minshawi |
| 18 | Khalifa Al-Tunaiji |
| 19 | Mishari Rashid al-Afasy |
| 20 | Mahmoud Khalil Al-Husary |
| 21 | Abdul Basit Abdul Samad |
| 22 | Abdul Rahman Al-Sudais |
| 23 | Mohamed al-Tablawi |
| 24 | Mahmoud Khalil Al-Husary (Mujawwad) |
| 25 | Maher Al-Muaiqly |
| 26 | Abdul Basit Abdul Samad (Mujawwad) |
| 27 | Abu Bakr al-Shatri |
| 28 | Saad al-Ghamdi |

*Note: IDs 2, 13-28 are "gapped mode" (pause between verses). The tool uses this mode by default.*

---

### Command Cheat Sheet

Copy-paste these templates and just change the numbers/names:

**Basic Instagram video:**
```bash
cd ~/Documents/Quran_Video_test/quran-video-maker-ffmpeg
./build/qvm [SURAH] [START] [END] --output ~/Desktop/[NAME].mp4 --width 720 --height 1280
```

**With dynamic backgrounds:**
```bash
cd ~/Documents/Quran_Video_test/quran-video-maker-ffmpeg
./build/qvm [SURAH] [START] [END] --output ~/Desktop/[NAME].mp4 --width 720 --height 1280 --enable-dynamic-bg --local-video-dir ~/Desktop/bg_videos
```

**Different reciter + backgrounds:**
```bash
cd ~/Documents/Quran_Video_test/quran-video-maker-ffmpeg
./build/qvm [SURAH] [START] [END] --reciter [ID] --output ~/Desktop/[NAME].mp4 --width 720 --height 1280 --enable-dynamic-bg --local-video-dir ~/Desktop/bg_videos
```

**Get variety with seed:**
```bash
cd ~/Documents/Quran_Video_test/quran-video-maker-ffmpeg
./build/qvm [SURAH] [START] [END] --seed [NUMBER] --output ~/Desktop/[NAME].mp4 --width 720 --height 1280 --enable-dynamic-bg --local-video-dir ~/Desktop/bg_videos
```

**High quality final version:**
```bash
cd ~/Documents/Quran_Video_test/quran-video-maker-ffmpeg
./build/qvm [SURAH] [START] [END] --quality-profile max --output ~/Desktop/[NAME].mp4 --width 720 --height 1280 --enable-dynamic-bg --local-video-dir ~/Desktop/bg_videos
```

---

### Common Surahs for Instagram (30s - 3min)

| Surah | Verses | Length | Description |
|-------|--------|--------|-------------|
| 1 | 1-7 | ~30s | Al-Fatiha (The Opening) |
| 93 | 1-11 | ~45s | Ad-Duha (The Morning) |
| 94 | 1-8 | ~30s | Ash-Sharh (Comfort) |
| 103 | 1-3 | ~15s | Al-Asr (Time) |
| 108 | 1-3 | ~15s | Al-Kawthar (Abundance) |
| 112 | 1-4 | ~20s | Al-Ikhlas (Sincerity) |
| 113 | 1-5 | ~20s | Al-Falaq (Daybreak) |
| 114 | 1-6 | ~20s | An-Nas (Mankind) |
| 55 | 1-16 | ~2min | Ar-Rahman (first portion) |
| 67 | 1-15 | ~2.5min | Al-Mulk (first portion) |
| 36 | 1-12 | ~2min | Ya-Sin (first portion) |

---

### Useful Terminal Shortcuts

- **Up Arrow:** Brings back your last command (great for changing just the seed number!)
- **Cmd + K:** Clear terminal screen
- **Cmd + T:** Open new terminal tab
- **Ctrl + C:** Stop/cancel current render

---

### File Locations Quick Reference

| What | Where |
|------|-------|
| Project folder | `~/Documents/Quran_Video_test/quran-video-maker-ffmpeg` |
| Background videos | `~/Desktop/bg_videos/` |
| Theme settings | `metadata/surah-themes.json` (inside project folder) |
| Output videos | `~/Desktop/` (or wherever you specify with `--output`) |

---

### Troubleshooting

**"Command not found"**
- Make sure you're in the project folder first: `cd ~/Documents/Quran_Video_test/quran-video-maker-ffmpeg`

**"No such file or directory" for bg_videos**
- Check the path: `--local-video-dir ~/Desktop/bg_videos` (make sure it matches where your folders are)

**Background videos not matching the theme**
- Edit `metadata/surah-themes.json` to assign themes to that surah (see Section 4)

**Same clips keep appearing**
- Use different `--seed` numbers: `--seed 1`, `--seed 2`, `--seed 3`, etc.

**Video renders slowly**
- Use `--quality-profile speed` for faster previews
- Final versions will take longer but look better

---

*Created with love for sharing the beauty of the Quran*
