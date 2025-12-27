#include "proccli/normalizer.h"

#include "proccli/utils.h"

namespace proccli {

DiagnosticsSnapshot normalizeDiagnostics(const RawArtifacts &artifacts, const TargetInfo &target,
                                         const std::vector<CollectorResult> &collector_results) {
  DiagnosticsSnapshot snapshot;
  snapshot.target = target;
  if (artifacts.ps_output) {
    snapshot.processes = PsCollector::parse(*artifacts.ps_output);
  }
  if (artifacts.meminfo) {
    snapshot.system.meminfo = ProcfsCollector::parseMemInfo(*artifacts.meminfo);
  }
  if (artifacts.loadavg) {
    snapshot.system.loadavg = ProcfsCollector::parseLoadAvg(*artifacts.loadavg);
  }
  for (const auto &entry : artifacts.proc_io) {
    auto io = ProcfsCollector::parseIo(entry.first, entry.second);
    if (io) {
      snapshot.io.push_back(*io);
    }
  }
  if (artifacts.valgrind_output) {
    snapshot.valgrind = ValgrindCollector::parse(*artifacts.valgrind_output);
  }
  if (artifacts.perf_output) {
    snapshot.perf = PerfCollector::parse(*artifacts.perf_output);
  }
  if (artifacts.strace_output) {
    snapshot.strace = StraceCollector::parse(*artifacts.strace_output);
  }
  snapshot.timing.captured_at = isoTimestamp();

  for (const auto &collector : collector_results) {
    CollectorStatus status{collector.name, collector.status, collector.error};
    snapshot.quality.collectors.push_back(status);
  }

  return snapshot;
}

} // namespace proccli
