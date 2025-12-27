#pragma once

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace proccli {

struct TargetInfo {
  std::optional<int> pid;
  std::optional<std::string> command;
};

struct LoadAvg {
  double one = 0.0;
  double five = 0.0;
  double fifteen = 0.0;
};

struct MemInfo {
  int mem_total_kb = 0;
  int mem_free_kb = 0;
  int mem_available_kb = 0;
};

struct SystemInfo {
  std::optional<LoadAvg> loadavg;
  std::optional<MemInfo> meminfo;
};

struct ProcessInfo {
  int pid = 0;
  int ppid = 0;
  std::string cmd;
  int rss_kb = 0;
  int vsz_kb = 0;
  double cpu_percent = 0.0;
  double mem_percent = 0.0;
  std::string etime;
};

struct ValgrindError {
  std::string kind;
  int count = 0;
};

struct LeakSummary {
  int definitely_lost_kb = 0;
  int indirectly_lost_kb = 0;
  int possibly_lost_kb = 0;
  int still_reachable_kb = 0;
};

struct ValgrindReport {
  std::vector<ValgrindError> errors;
  std::optional<LeakSummary> leak_summary;
};

struct PerfHotspot {
  std::string symbol;
  double percent = 0.0;
};

struct PerfReport {
  std::vector<PerfHotspot> hotspots;
};

struct StraceSyscall {
  std::string name;
  int count = 0;
  double time_ms = 0.0;
};

struct StraceSlowSyscall {
  std::string name;
  double duration_ms = 0.0;
};

struct StraceReport {
  std::vector<StraceSyscall> top_syscalls;
  std::vector<StraceSlowSyscall> slow_syscalls;
};

struct IoStats {
  int pid = 0;
  long long read_bytes = 0;
  long long write_bytes = 0;
};

struct TimingInfo {
  std::string captured_at;
};

struct CollectorStatus {
  std::string name;
  std::string status;
  std::optional<std::string> error;
};

struct QualityInfo {
  std::vector<CollectorStatus> collectors;
};

struct DiagnosticsSnapshot {
  std::string version = "0.1";
  TargetInfo target;
  SystemInfo system;
  std::vector<ProcessInfo> processes;
  std::optional<ValgrindReport> valgrind;
  std::optional<PerfReport> perf;
  std::optional<StraceReport> strace;
  std::vector<IoStats> io;
  TimingInfo timing;
  QualityInfo quality;
};

void to_json(nlohmann::json &j, const TargetInfo &info);
void to_json(nlohmann::json &j, const LoadAvg &info);
void to_json(nlohmann::json &j, const MemInfo &info);
void to_json(nlohmann::json &j, const SystemInfo &info);
void to_json(nlohmann::json &j, const ProcessInfo &info);
void to_json(nlohmann::json &j, const ValgrindError &info);
void to_json(nlohmann::json &j, const LeakSummary &info);
void to_json(nlohmann::json &j, const ValgrindReport &info);
void to_json(nlohmann::json &j, const PerfHotspot &info);
void to_json(nlohmann::json &j, const PerfReport &info);
void to_json(nlohmann::json &j, const StraceSyscall &info);
void to_json(nlohmann::json &j, const StraceSlowSyscall &info);
void to_json(nlohmann::json &j, const StraceReport &info);
void to_json(nlohmann::json &j, const IoStats &info);
void to_json(nlohmann::json &j, const TimingInfo &info);
void to_json(nlohmann::json &j, const CollectorStatus &info);
void to_json(nlohmann::json &j, const QualityInfo &info);
void to_json(nlohmann::json &j, const DiagnosticsSnapshot &info);

DiagnosticsSnapshot snapshotFromJson(const nlohmann::json &j);

} // namespace proccli
