#include <gtest/gtest.h>

#include "proccli/normalizer.h"

TEST(NormalizerTest, BuildsSnapshotFromArtifacts) {
  proccli::RawArtifacts artifacts;
  artifacts.ps_output = "123 1 /usr/bin/bash 2048 4096 0.1 0.2 00:00:05\n";
  artifacts.meminfo = "MemTotal:       16384 kB\nMemFree:         4096 kB\nMemAvailable:    8192 kB\n";
  artifacts.loadavg = "0.10 0.20 0.30 1/234 567\n";
  artifacts.proc_io.push_back({123, "read_bytes: 100\nwrite_bytes: 200\n"});

  proccli::TargetInfo target;
  target.pid = 123;

  std::vector<proccli::CollectorResult> collectors = {
      {"ps", "ok", std::nullopt}, {"proc", "ok", std::nullopt}};

  auto snapshot = proccli::normalizeDiagnostics(artifacts, target, collectors);
  EXPECT_EQ(snapshot.target.pid, 123);
  ASSERT_FALSE(snapshot.processes.empty());
  EXPECT_EQ(snapshot.processes[0].cmd, "/usr/bin/bash");
  ASSERT_TRUE(snapshot.system.meminfo.has_value());
  EXPECT_EQ(snapshot.system.meminfo->mem_total_kb, 16384);
  ASSERT_TRUE(snapshot.system.loadavg.has_value());
  EXPECT_DOUBLE_EQ(snapshot.system.loadavg->five, 0.20);
  ASSERT_FALSE(snapshot.io.empty());
  EXPECT_EQ(snapshot.io[0].read_bytes, 100);
  ASSERT_EQ(snapshot.quality.collectors.size(), 2u);
}
