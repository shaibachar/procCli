#include "proccli/collectors.h"
#include "proccli/utils.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <map>
#include <regex>
#include <sstream>

#include <spdlog/spdlog.h>

namespace proccli {

CommandResult runCommand(const std::string &command) {
  CommandResult result;
  std::array<char, 256> buffer{};
  std::string output;
  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe) {
    result.exit_code = 1;
    return result;
  }
  while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
    output += buffer.data();
  }
  int code = pclose(pipe);
  result.exit_code = code;
  result.output = output;
  return result;
}

CommandResult PsCollector::collect() {
  return runCommand("ps -eo pid,ppid,cmd,rss,vsz,pcpu,pmem,etime --no-headers");
}

std::vector<ProcessInfo> PsCollector::parse(const std::string &output) {
  std::vector<ProcessInfo> processes;
  std::istringstream stream(output);
  std::string line;
  while (std::getline(stream, line)) {
    if (line.empty()) {
      continue;
    }
    std::istringstream line_stream(line);
    ProcessInfo info;
    if (!(line_stream >> info.pid >> info.ppid)) {
      continue;
    }
    std::string cmd_part;
    std::vector<std::string> cmd_tokens;
    while (line_stream >> cmd_part) {
      cmd_tokens.push_back(cmd_part);
    }
    if (cmd_tokens.size() < 5) {
      continue;
    }
    info.etime = cmd_tokens.back();
    cmd_tokens.pop_back();
    info.mem_percent = std::stod(cmd_tokens.back());
    cmd_tokens.pop_back();
    info.cpu_percent = std::stod(cmd_tokens.back());
    cmd_tokens.pop_back();
    info.vsz_kb = std::stoi(cmd_tokens.back());
    cmd_tokens.pop_back();
    info.rss_kb = std::stoi(cmd_tokens.back());
    cmd_tokens.pop_back();
    std::ostringstream cmd_stream;
    for (size_t i = 0; i < cmd_tokens.size(); ++i) {
      if (i > 0) {
        cmd_stream << ' ';
      }
      cmd_stream << cmd_tokens[i];
    }
    info.cmd = cmd_stream.str();
    processes.push_back(info);
  }
  return processes;
}

CommandResult ProcfsCollector::collectMemInfo() {
  return runCommand("cat /proc/meminfo");
}

CommandResult ProcfsCollector::collectLoadAvg() {
  return runCommand("cat /proc/loadavg");
}

std::optional<CommandResult> ProcfsCollector::collectStatus(int pid) {
  std::string path = "/proc/" + std::to_string(pid) + "/status";
  std::string content = readFile(path);
  if (content.empty()) {
    return std::nullopt;
  }
  return CommandResult{0, content};
}

std::optional<CommandResult> ProcfsCollector::collectIo(int pid) {
  std::string path = "/proc/" + std::to_string(pid) + "/io";
  std::string content = readFile(path);
  if (content.empty()) {
    return std::nullopt;
  }
  return CommandResult{0, content};
}

std::optional<MemInfo> ProcfsCollector::parseMemInfo(const std::string &content) {
  if (content.empty()) {
    return std::nullopt;
  }
  MemInfo info;
  std::istringstream stream(content);
  std::string key;
  long value = 0;
  std::string unit;
  while (stream >> key >> value >> unit) {
    if (key == "MemTotal:") {
      info.mem_total_kb = static_cast<int>(value);
    } else if (key == "MemFree:") {
      info.mem_free_kb = static_cast<int>(value);
    } else if (key == "MemAvailable:") {
      info.mem_available_kb = static_cast<int>(value);
    }
  }
  return info;
}

std::optional<LoadAvg> ProcfsCollector::parseLoadAvg(const std::string &content) {
  if (content.empty()) {
    return std::nullopt;
  }
  std::istringstream stream(content);
  LoadAvg info;
  if (!(stream >> info.one >> info.five >> info.fifteen)) {
    return std::nullopt;
  }
  return info;
}

std::optional<IoStats> ProcfsCollector::parseIo(int pid, const std::string &content) {
  if (content.empty()) {
    return std::nullopt;
  }
  IoStats stats;
  stats.pid = pid;
  std::istringstream stream(content);
  std::string key;
  long long value = 0;
  while (stream >> key >> value) {
    if (key == "read_bytes:") {
      stats.read_bytes = value;
    } else if (key == "write_bytes:") {
      stats.write_bytes = value;
    }
  }
  return stats;
}

std::optional<ValgrindReport> ValgrindCollector::parse(const std::string &output) {
  if (output.empty()) {
    return std::nullopt;
  }
  ValgrindReport report;
  std::regex error_summary(R"(ERROR SUMMARY: (\d+) errors)" );
  std::smatch match;
  if (std::regex_search(output, match, error_summary)) {
    report.errors.push_back({"summary", std::stoi(match[1])});
  }
  std::regex leak_line(R"((definitely lost|indirectly lost|possibly lost|still reachable):\s+([0-9,]+) bytes)" );
  auto begin = std::sregex_iterator(output.begin(), output.end(), leak_line);
  auto end = std::sregex_iterator();
  LeakSummary summary;
  bool found = false;
  for (auto it = begin; it != end; ++it) {
    std::string kind = (*it)[1];
    std::string bytes_str = (*it)[2];
    bytes_str.erase(std::remove(bytes_str.begin(), bytes_str.end(), ','), bytes_str.end());
    int kb = static_cast<int>(std::stoll(bytes_str) / 1024);
    if (kind == "definitely lost") {
      summary.definitely_lost_kb = kb;
    } else if (kind == "indirectly lost") {
      summary.indirectly_lost_kb = kb;
    } else if (kind == "possibly lost") {
      summary.possibly_lost_kb = kb;
    } else if (kind == "still reachable") {
      summary.still_reachable_kb = kb;
    }
    found = true;
  }
  if (found) {
    report.leak_summary = summary;
  }
  return report;
}

std::optional<PerfReport> PerfCollector::parse(const std::string &output) {
  if (output.empty()) {
    return std::nullopt;
  }
  PerfReport report;
  std::regex line_regex(R"(^\s*([0-9]+\.[0-9]+)%\s+.*\s+(\S+)$)" );
  std::istringstream stream(output);
  std::string line;
  while (std::getline(stream, line)) {
    std::smatch match;
    if (std::regex_search(line, match, line_regex)) {
      report.hotspots.push_back({match[2], std::stod(match[1])});
    }
  }
  return report;
}

std::optional<StraceReport> StraceCollector::parse(const std::string &output) {
  if (output.empty()) {
    return std::nullopt;
  }
  StraceReport report;
  std::regex syscall_regex(R"((\w+)\(.*\)\s+=\s+.*<([0-9.]+)>)" );
  std::istringstream stream(output);
  std::string line;
  std::map<std::string, std::pair<int, double>> stats;
  std::vector<StraceSlowSyscall> slow;
  while (std::getline(stream, line)) {
    std::smatch match;
    if (std::regex_search(line, match, syscall_regex)) {
      std::string name = match[1];
      double duration = std::stod(match[2]) * 1000.0;
      auto &entry = stats[name];
      entry.first += 1;
      entry.second += duration;
      slow.push_back({name, duration});
    }
  }
  std::vector<StraceSyscall> syscalls;
  syscalls.reserve(stats.size());
  for (const auto &item : stats) {
    syscalls.push_back({item.first, item.second.first, item.second.second});
  }
  std::sort(syscalls.begin(), syscalls.end(),
            [](const StraceSyscall &a, const StraceSyscall &b) { return a.count > b.count; });
  if (syscalls.size() > 5) {
    syscalls.resize(5);
  }
  std::sort(slow.begin(), slow.end(),
            [](const StraceSlowSyscall &a, const StraceSlowSyscall &b) {
              return a.duration_ms > b.duration_ms;
            });
  if (slow.size() > 5) {
    slow.resize(5);
  }
  report.top_syscalls = syscalls;
  report.slow_syscalls = slow;
  return report;
}

} // namespace proccli
