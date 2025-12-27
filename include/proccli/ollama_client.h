#pragma once

#include <string>

#include "proccli/diagnostics.h"

namespace proccli {

struct OllamaResult {
  bool ok = false;
  std::string response;
  std::string error;
};

class OllamaClient {
 public:
  OllamaResult analyze(const DiagnosticsSnapshot &snapshot, const std::string &model) const;
};

} // namespace proccli
