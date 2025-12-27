#pragma once

#include <optional>
#include <string>
#include <vector>

#include "proccli/diagnostics.h"

namespace proccli {

struct CommandResult {
  int exit_code = 0;
  std::string output;
};

struct CollectorResult {
  std::string name;
  std::string status;
  std::optional<std::string> error;
};

struct RawArtifacts {
  std::optional<std::string> ps_output;
  std::optional<std::string> meminfo;
  std::optional<std::string> loadavg;
  std::vector<std::pair<int, std::string>> proc_status;
  std::vector<std::pair<int, std::string>> proc_io;
  std::optional<std::string> valgrind_output;
  std::optional<std::string> perf_output;
  std::optional<std::string> strace_output;
};

class PsCollector {
 public:
  static std::vector<ProcessInfo> parse(const std::string &output);
  CommandResult collect();
};

class ProcfsCollector {
 public:
  CommandResult collectMemInfo();
  CommandResult collectLoadAvg();
  std::optional<CommandResult> collectStatus(int pid);
  std::optional<CommandResult> collectIo(int pid);

  static std::optional<MemInfo> parseMemInfo(const std::string &content);
  static std::optional<LoadAvg> parseLoadAvg(const std::string &content);
  static std::optional<IoStats> parseIo(int pid, const std::string &content);
};

class ValgrindCollector {
 public:
  static std::optional<ValgrindReport> parse(const std::string &output);
};

class PerfCollector {
 public:
  static std::optional<PerfReport> parse(const std::string &output);
};

class StraceCollector {
 public:
  static std::optional<StraceReport> parse(const std::string &output);
};

CommandResult runCommand(const std::string &command);

} // namespace proccli
