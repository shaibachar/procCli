#include "proccli/diagnostics.h"

namespace proccli {

void to_json(nlohmann::json &j, const TargetInfo &info) {
  j = nlohmann::json::object();
  if (info.pid) {
    j["pid"] = *info.pid;
  }
  if (info.command) {
    j["command"] = *info.command;
  }
}

void to_json(nlohmann::json &j, const LoadAvg &info) {
  j = nlohmann::json{{"one", info.one}, {"five", info.five}, {"fifteen", info.fifteen}};
}

void to_json(nlohmann::json &j, const MemInfo &info) {
  j = nlohmann::json{{"mem_total_kb", info.mem_total_kb},
                     {"mem_free_kb", info.mem_free_kb},
                     {"mem_available_kb", info.mem_available_kb}};
}

void to_json(nlohmann::json &j, const SystemInfo &info) {
  j = nlohmann::json::object();
  if (info.loadavg) {
    j["loadavg"] = *info.loadavg;
  }
  if (info.meminfo) {
    j["meminfo"] = *info.meminfo;
  }
}

void to_json(nlohmann::json &j, const ProcessInfo &info) {
  j = nlohmann::json{{"pid", info.pid},
                     {"ppid", info.ppid},
                     {"cmd", info.cmd},
                     {"rss_kb", info.rss_kb},
                     {"vsz_kb", info.vsz_kb},
                     {"cpu_percent", info.cpu_percent},
                     {"mem_percent", info.mem_percent},
                     {"etime", info.etime}};
}

void to_json(nlohmann::json &j, const ValgrindError &info) {
  j = nlohmann::json{{"kind", info.kind}, {"count", info.count}};
}

void to_json(nlohmann::json &j, const LeakSummary &info) {
  j = nlohmann::json{{"definitely_lost_kb", info.definitely_lost_kb},
                     {"indirectly_lost_kb", info.indirectly_lost_kb},
                     {"possibly_lost_kb", info.possibly_lost_kb},
                     {"still_reachable_kb", info.still_reachable_kb}};
}

void to_json(nlohmann::json &j, const ValgrindReport &info) {
  j = nlohmann::json{{"errors", info.errors}};
  if (info.leak_summary) {
    j["leak_summary"] = *info.leak_summary;
  }
}

void to_json(nlohmann::json &j, const PerfHotspot &info) {
  j = nlohmann::json{{"symbol", info.symbol}, {"percent", info.percent}};
}

void to_json(nlohmann::json &j, const PerfReport &info) {
  j = nlohmann::json{{"hotspots", info.hotspots}};
}

void to_json(nlohmann::json &j, const StraceSyscall &info) {
  j = nlohmann::json{{"name", info.name}, {"count", info.count}, {"time_ms", info.time_ms}};
}

void to_json(nlohmann::json &j, const StraceSlowSyscall &info) {
  j = nlohmann::json{{"name", info.name}, {"duration_ms", info.duration_ms}};
}

void to_json(nlohmann::json &j, const StraceReport &info) {
  j = nlohmann::json{{"top_syscalls", info.top_syscalls}, {"slow_syscalls", info.slow_syscalls}};
}

void to_json(nlohmann::json &j, const IoStats &info) {
  j = nlohmann::json{{"pid", info.pid},
                     {"read_bytes", info.read_bytes},
                     {"write_bytes", info.write_bytes}};
}

void to_json(nlohmann::json &j, const TimingInfo &info) {
  j = nlohmann::json{{"captured_at", info.captured_at}};
}

void to_json(nlohmann::json &j, const CollectorStatus &info) {
  j = nlohmann::json{{"name", info.name}, {"status", info.status}};
  if (info.error) {
    j["error"] = *info.error;
  }
}

void to_json(nlohmann::json &j, const QualityInfo &info) {
  j = nlohmann::json{{"collectors", info.collectors}};
}

void to_json(nlohmann::json &j, const DiagnosticsSnapshot &info) {
  j = nlohmann::json{{"version", info.version},
                     {"target", info.target},
                     {"system", info.system},
                     {"processes", info.processes},
                     {"io", info.io},
                     {"timing", info.timing},
                     {"quality", info.quality}};
  if (info.valgrind) {
    j["valgrind"] = *info.valgrind;
  }
  if (info.perf) {
    j["perf"] = *info.perf;
  }
  if (info.strace) {
    j["strace"] = *info.strace;
  }
}

DiagnosticsSnapshot snapshotFromJson(const nlohmann::json &j) {
  DiagnosticsSnapshot snapshot;
  snapshot.version = j.value("version", "0.1");
  const auto &target = j.at("target");
  if (target.contains("pid")) {
    snapshot.target.pid = target.at("pid").get<int>();
  }
  if (target.contains("command")) {
    snapshot.target.command = target.at("command").get<std::string>();
  }
  if (j.contains("system")) {
    const auto &sys = j.at("system");
    if (sys.contains("loadavg")) {
      snapshot.system.loadavg = LoadAvg{sys.at("loadavg").value("one", 0.0),
                                        sys.at("loadavg").value("five", 0.0),
                                        sys.at("loadavg").value("fifteen", 0.0)};
    }
    if (sys.contains("meminfo")) {
      snapshot.system.meminfo =
          MemInfo{sys.at("meminfo").value("mem_total_kb", 0),
                  sys.at("meminfo").value("mem_free_kb", 0),
                  sys.at("meminfo").value("mem_available_kb", 0)};
    }
  }
  if (j.contains("processes")) {
    for (const auto &proc : j.at("processes")) {
      ProcessInfo info;
      info.pid = proc.value("pid", 0);
      info.ppid = proc.value("ppid", 0);
      info.cmd = proc.value("cmd", "");
      info.rss_kb = proc.value("rss_kb", 0);
      info.vsz_kb = proc.value("vsz_kb", 0);
      info.cpu_percent = proc.value("cpu_percent", 0.0);
      info.mem_percent = proc.value("mem_percent", 0.0);
      info.etime = proc.value("etime", "");
      snapshot.processes.push_back(info);
    }
  }
  if (j.contains("valgrind")) {
    ValgrindReport vg;
    if (j.at("valgrind").contains("errors")) {
      for (const auto &err : j.at("valgrind").at("errors")) {
        vg.errors.push_back({err.value("kind", ""), err.value("count", 0)});
      }
    }
    if (j.at("valgrind").contains("leak_summary")) {
      const auto &ls = j.at("valgrind").at("leak_summary");
      vg.leak_summary = LeakSummary{ls.value("definitely_lost_kb", 0),
                                    ls.value("indirectly_lost_kb", 0),
                                    ls.value("possibly_lost_kb", 0),
                                    ls.value("still_reachable_kb", 0)};
    }
    snapshot.valgrind = vg;
  }
  if (j.contains("perf")) {
    PerfReport pr;
    for (const auto &hotspot : j.at("perf").at("hotspots")) {
      pr.hotspots.push_back({hotspot.value("symbol", ""), hotspot.value("percent", 0.0)});
    }
    snapshot.perf = pr;
  }
  if (j.contains("strace")) {
    StraceReport sr;
    for (const auto &syscall : j.at("strace").at("top_syscalls")) {
      sr.top_syscalls.push_back(
          {syscall.value("name", ""), syscall.value("count", 0), syscall.value("time_ms", 0.0)});
    }
    for (const auto &slow : j.at("strace").at("slow_syscalls")) {
      sr.slow_syscalls.push_back({slow.value("name", ""), slow.value("duration_ms", 0.0)});
    }
    snapshot.strace = sr;
  }
  if (j.contains("io")) {
    for (const auto &io : j.at("io")) {
      snapshot.io.push_back({io.value("pid", 0), io.value("read_bytes", 0LL),
                             io.value("write_bytes", 0LL)});
    }
  }
  if (j.contains("timing")) {
    snapshot.timing.captured_at = j.at("timing").value("captured_at", "");
  }
  if (j.contains("quality")) {
    for (const auto &collector : j.at("quality").at("collectors")) {
      CollectorStatus status;
      status.name = collector.value("name", "");
      status.status = collector.value("status", "");
      if (collector.contains("error")) {
        status.error = collector.at("error").get<std::string>();
      }
      snapshot.quality.collectors.push_back(status);
    }
  }
  return snapshot;
}

} // namespace proccli
