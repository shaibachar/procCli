#include <gtest/gtest.h>

#include "proccli/collectors.h"

TEST(PsCollectorTest, ParsesProcessLine) {
  std::string output = "123 1 /usr/bin/bash 2048 4096 0.1 0.2 00:00:05\n";
  auto result = proccli::PsCollector::parse(output);
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(result[0].pid, 123);
  EXPECT_EQ(result[0].ppid, 1);
  EXPECT_EQ(result[0].cmd, "/usr/bin/bash");
  EXPECT_EQ(result[0].rss_kb, 2048);
  EXPECT_EQ(result[0].vsz_kb, 4096);
  EXPECT_DOUBLE_EQ(result[0].cpu_percent, 0.1);
  EXPECT_DOUBLE_EQ(result[0].mem_percent, 0.2);
  EXPECT_EQ(result[0].etime, "00:00:05");
}

TEST(ProcfsCollectorTest, ParsesMemInfo) {
  std::string input = "MemTotal:       16384 kB\nMemFree:         4096 kB\nMemAvailable:    8192 kB\n";
  auto info = proccli::ProcfsCollector::parseMemInfo(input);
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->mem_total_kb, 16384);
  EXPECT_EQ(info->mem_free_kb, 4096);
  EXPECT_EQ(info->mem_available_kb, 8192);
}

TEST(ValgrindCollectorTest, ParsesLeakSummary) {
  std::string output =
      "ERROR SUMMARY: 2 errors from 2 contexts (suppressed: 0 from 0)\n"
      "definitely lost: 1024 bytes in 2 blocks\n"
      "indirectly lost: 2048 bytes in 1 blocks\n"
      "possibly lost: 0 bytes in 0 blocks\n"
      "still reachable: 4096 bytes in 4 blocks\n";
  auto report = proccli::ValgrindCollector::parse(output);
  ASSERT_TRUE(report.has_value());
  ASSERT_TRUE(report->leak_summary.has_value());
  EXPECT_EQ(report->errors[0].count, 2);
  EXPECT_EQ(report->leak_summary->definitely_lost_kb, 1);
  EXPECT_EQ(report->leak_summary->indirectly_lost_kb, 2);
  EXPECT_EQ(report->leak_summary->still_reachable_kb, 4);
}

TEST(PerfCollectorTest, ParsesHotspots) {
  std::string output = "  12.34%  app  [.] main\n   5.00%  app  [.] worker\n";
  auto report = proccli::PerfCollector::parse(output);
  ASSERT_TRUE(report.has_value());
  ASSERT_EQ(report->hotspots.size(), 2u);
  EXPECT_EQ(report->hotspots[0].symbol, "main");
  EXPECT_DOUBLE_EQ(report->hotspots[0].percent, 12.34);
}

TEST(StraceCollectorTest, ParsesSyscalls) {
  std::string output =
      "openat(AT_FDCWD, \"/etc/hosts\", O_RDONLY) = 3 <0.000123>\n"
      "read(3, \"data\", 4) = 4 <0.010000>\n"
      "read(3, \"data\", 4) = 4 <0.020000>\n";
  auto report = proccli::StraceCollector::parse(output);
  ASSERT_TRUE(report.has_value());
  ASSERT_FALSE(report->top_syscalls.empty());
  EXPECT_EQ(report->top_syscalls[0].name, "read");
  EXPECT_EQ(report->top_syscalls[0].count, 2);
  EXPECT_FALSE(report->slow_syscalls.empty());
}
