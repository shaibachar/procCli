#include "proccli/utils.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace proccli {

std::string readFile(const std::string &path) {
  std::ifstream file(path);
  if (!file) {
    return "";
  }
  std::ostringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

void writeFile(const std::string &path, const std::string &content) {
  std::filesystem::create_directories(std::filesystem::path(path).parent_path());
  std::ofstream file(path);
  file << content;
}

std::string isoTimestamp() {
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
  gmtime_r(&time, &tm);
  std::ostringstream stream;
  stream << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
  return stream.str();
}

std::string makeArtifactsDir(const std::string &base) {
  if (!base.empty()) {
    std::filesystem::create_directories(base + "/raw");
    return base;
  }
  std::string dir = "artifacts/" + isoTimestamp();
  std::filesystem::create_directories(dir + "/raw");
  return dir;
}

} // namespace proccli
