#pragma once

#include <string>

namespace proccli {

std::string readFile(const std::string &path);
void writeFile(const std::string &path, const std::string &content);
std::string isoTimestamp();
std::string makeArtifactsDir(const std::string &base);

} // namespace proccli
