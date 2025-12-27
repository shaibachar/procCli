#pragma once

#include <string>

#include "proccli/diagnostics.h"

namespace proccli {

std::string renderReport(const std::string &analysis, const DiagnosticsSnapshot &snapshot);

} // namespace proccli
