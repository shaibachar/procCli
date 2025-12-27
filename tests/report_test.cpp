#include <gtest/gtest.h>

#include "proccli/report.h"

TEST(ReportTest, IncludesLimitations) {
  proccli::DiagnosticsSnapshot snapshot;
  snapshot.quality.collectors.push_back({"ps", "ok", std::nullopt});
  snapshot.quality.collectors.push_back({"perf", "failed", std::string("missing")});

  auto report = proccli::renderReport("Findings content", snapshot);
  EXPECT_NE(report.find("Findings"), std::string::npos);
  EXPECT_NE(report.find("perf"), std::string::npos);
  EXPECT_NE(report.find("missing"), std::string::npos);
}
