#pragma once

#include <optional>
#include <string>

#include "proccli/collectors.h"
#include "proccli/diagnostics.h"

namespace proccli {

DiagnosticsSnapshot normalizeDiagnostics(const RawArtifacts &artifacts, const TargetInfo &target,
                                         const std::vector<CollectorResult> &collector_results);

} // namespace proccli
