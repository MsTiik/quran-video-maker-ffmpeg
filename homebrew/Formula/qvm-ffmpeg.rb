class QvmFfmpeg < Formula
  desc "Quran Video Maker (FFmpeg)"
  homepage "https://github.com/ashaltu/quran-video-maker-ffmpeg"
  url "https://github.com/ashaltu/quran-video-maker-ffmpeg/archive/refs/tags/v0.0.0.tar.gz"
  sha256 "0"

  depends_on "cmake" => :build
  depends_on "pkg-config" => :build
  depends_on "ffmpeg"
  depends_on "freetype"
  depends_on "harfbuzz"
  depends_on "cpr"
  depends_on "nlohmann-json"
  depends_on "cxxopts"

  def install
    system "cmake", "-S", ".", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end
end
