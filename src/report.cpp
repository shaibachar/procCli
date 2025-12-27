#include "proccli/report.h"

#include <sstream>

namespace proccli {

std::string renderReport(const std::string &analysis, const DiagnosticsSnapshot &snapshot) {
  std::ostringstream output;
  output << "Findings\n";
  output << "========\n";
  output << analysis << "\n\n";
  output << "Recommendations\n";
  output << "===============\n";
  output << "Review the findings above and prioritize actions based on severity and effort.\n\n";
  output << "Limitations\n";
  output << "===========\n";
  bool any = false;
  for (const auto &collector : snapshot.quality.collectors) {
    if (collector.status != "ok") {
      any = true;
      output << "- " << collector.name << ": " << collector.status;
      if (collector.error) {
        output << " (" << *collector.error << ")";
      }
      output << "\n";
    }
  }
  if (!any) {
    output << "- None\n";
  }
  return output.str();
}

} // namespace proccli
