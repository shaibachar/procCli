#include "proccli/ollama_client.h"

#include <nlohmann/json.hpp>

#include "proccli/collectors.h"

namespace proccli {

namespace {
std::string extractResponse(const std::string &payload) {
  auto pos = payload.find("\"response\"");
  if (pos == std::string::npos) {
    return payload;
  }
  auto start = payload.find('"', pos + 10);
  if (start == std::string::npos) {
    return payload;
  }
  start = payload.find('"', start + 1);
  if (start == std::string::npos) {
    return payload;
  }
  auto end = payload.find('"', start + 1);
  if (end == std::string::npos) {
    return payload;
  }
  return payload.substr(start + 1, end - start - 1);
}

std::string escapeForShell(const std::string &value) {
  std::string escaped;
  escaped.reserve(value.size());
  for (char c : value) {
    if (c == '"') {
      escaped += "\\\"";
    } else if (c == '\\') {
      escaped += "\\\\";
    } else if (c == '\n') {
      escaped += "\\n";
    } else {
      escaped += c;
    }
  }
  return escaped;
}
} // namespace

OllamaResult OllamaClient::analyze(const DiagnosticsSnapshot &snapshot, const std::string &model) const {
  nlohmann::json payload;
  payload["model"] = model;
  payload["prompt"] =
      "Analyze the following diagnostics JSON and provide findings and recommended actions.";
  payload["stream"] = false;
  payload["context"] = nlohmann::json::array();
  payload["input"] = snapshot;

  std::string payload_str = payload.dump();
  std::string command = "curl -s http://localhost:11434/api/generate -d \"" +
                        escapeForShell(payload_str) + "\"";
  auto result = runCommand(command);
  if (result.exit_code != 0 || result.output.empty()) {
    return {false,
            "Unable to reach local Ollama server. Provide guidance based on available diagnostics.",
            "Ollama server unavailable"};
  }
  return {true, extractResponse(result.output), ""};
}

} // namespace proccli
