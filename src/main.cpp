#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "proccli/collectors.h"
#include "proccli/diagnostics.h"
#include "proccli/normalizer.h"
#include "proccli/ollama_client.h"
#include "proccli/report.h"
#include "proccli/utils.h"

namespace proccli {

enum class CommandType { Run, Collect, Analyze, Report };

struct Options {
  CommandType command = CommandType::Run;
  std::optional<int> pid;
  std::optional<std::string> command_str;
  std::string output;
  std::string input;
  std::string format = "text";
  bool valgrind = true;
  bool ps = true;
  bool procfs = true;
  bool perf = true;
  bool strace = true;
  int strace_timeout = 10;
  int perf_duration = 10;
  std::string valgrind_tool = "memcheck";
  std::string model = "llama3";
};

void printUsage() {
  std::cout << "proccli [command] [options]\n\n"
            << "Commands: run, collect, analyze, report\n"
            << "Options: --pid <pid>, --command <cmd>, --output <path>, --input <path>, --format text|json\n";
}

std::optional<Options> parseArgs(int argc, char **argv, std::string &error) {
  Options options;
  int index = 1;
  if (index < argc) {
    std::string first = argv[index];
    if (first == "run" || first == "collect" || first == "analyze" || first == "report") {
      if (first == "collect") {
        options.command = CommandType::Collect;
      } else if (first == "analyze") {
        options.command = CommandType::Analyze;
      } else if (first == "report") {
        options.command = CommandType::Report;
      } else {
        options.command = CommandType::Run;
      }
      index++;
    }
  }
  while (index < argc) {
    std::string arg = argv[index];
    if (arg == "--pid" && index + 1 < argc) {
      options.pid = std::stoi(argv[++index]);
    } else if (arg == "--command" && index + 1 < argc) {
      options.command_str = argv[++index];
    } else if (arg == "--output" && index + 1 < argc) {
      options.output = argv[++index];
    } else if (arg == "--input" && index + 1 < argc) {
      options.input = argv[++index];
    } else if (arg == "--format" && index + 1 < argc) {
      options.format = argv[++index];
    } else if (arg == "--no-valgrind") {
      options.valgrind = false;
    } else if (arg == "--no-ps") {
      options.ps = false;
    } else if (arg == "--no-proc") {
      options.procfs = false;
    } else if (arg == "--no-perf") {
      options.perf = false;
    } else if (arg == "--no-strace") {
      options.strace = false;
    } else if (arg == "--strace-timeout" && index + 1 < argc) {
      options.strace_timeout = std::stoi(argv[++index]);
    } else if (arg == "--perf-duration" && index + 1 < argc) {
      options.perf_duration = std::stoi(argv[++index]);
    } else if (arg == "--valgrind-tool" && index + 1 < argc) {
      options.valgrind_tool = argv[++index];
    } else if (arg == "--model" && index + 1 < argc) {
      options.model = argv[++index];
    } else if (arg == "--help") {
      printUsage();
      return std::nullopt;
    } else {
      error = "Unknown argument: " + arg;
      return std::nullopt;
    }
    index++;
  }

  if (options.pid && options.command_str) {
    error = "--pid and --command are mutually exclusive";
    return std::nullopt;
  }
  if ((options.command == CommandType::Analyze || options.command == CommandType::Report) &&
      options.input.empty()) {
    error = "--input is required for analyze/report";
    return std::nullopt;
  }
  if ((options.command == CommandType::Run || options.command == CommandType::Collect) &&
      !options.pid && !options.command_str) {
    error = "--pid or --command is required";
    return std::nullopt;
  }
  return options;
}

int runCommandTarget(const std::string &command) {
  pid_t pid = fork();
  if (pid == 0) {
    execl("/bin/sh", "sh", "-c", command.c_str(), static_cast<char *>(nullptr));
    _exit(127);
  }
  return static_cast<int>(pid);
}

struct CollectedData {
  std::string artifact_dir;
  DiagnosticsSnapshot snapshot;
  std::vector<CollectorResult> collector_results;
  RawArtifacts artifacts;
};

CollectorResult recordCollector(const std::string &name, bool enabled, const std::string &output,
                                const std::string &error = "") {
  CollectorResult result;
  result.name = name;
  if (!enabled) {
    result.status = "disabled";
  } else if (!error.empty()) {
    result.status = "failed";
    result.error = error;
  } else {
    result.status = "ok";
  }
  return result;
}

CollectedData collect(const Options &options) {
  CollectedData data;
  data.artifact_dir = makeArtifactsDir(options.output);

  TargetInfo target;
  int target_pid = 0;
  if (options.pid) {
    target.pid = options.pid;
    target_pid = *options.pid;
  } else if (options.command_str) {
    target.command = options.command_str;
    target_pid = runCommandTarget(*options.command_str);
    target.pid = target_pid;
  }

  if (options.ps) {
    PsCollector collector;
    auto result = collector.collect();
    if (result.exit_code == 0) {
      data.artifacts.ps_output = result.output;
      writeFile(data.artifact_dir + "/raw/ps.txt", result.output);
      data.collector_results.push_back(recordCollector("ps", true, result.output));
    } else {
      data.collector_results.push_back(recordCollector("ps", true, "", "ps failed"));
    }
  } else {
    data.collector_results.push_back(recordCollector("ps", false, ""));
  }

  if (options.procfs) {
    ProcfsCollector proc;
    auto meminfo = proc.collectMemInfo();
    if (meminfo.exit_code == 0) {
      data.artifacts.meminfo = meminfo.output;
      writeFile(data.artifact_dir + "/raw/meminfo.txt", meminfo.output);
    }
    auto loadavg = proc.collectLoadAvg();
    if (loadavg.exit_code == 0) {
      data.artifacts.loadavg = loadavg.output;
      writeFile(data.artifact_dir + "/raw/loadavg.txt", loadavg.output);
    }
    if (target_pid > 0) {
      if (auto status = proc.collectStatus(target_pid)) {
        data.artifacts.proc_status.push_back({target_pid, status->output});
        writeFile(data.artifact_dir + "/raw/status.txt", status->output);
      }
      if (auto io = proc.collectIo(target_pid)) {
        data.artifacts.proc_io.push_back({target_pid, io->output});
        writeFile(data.artifact_dir + "/raw/io.txt", io->output);
      }
    }
    data.collector_results.push_back(recordCollector("proc", true, ""));
  } else {
    data.collector_results.push_back(recordCollector("proc", false, ""));
  }

  if (options.valgrind) {
    data.collector_results.push_back(
        recordCollector("valgrind", true, "", "valgrind execution not implemented"));
  } else {
    data.collector_results.push_back(recordCollector("valgrind", false, ""));
  }

  if (options.perf) {
    data.collector_results.push_back(
        recordCollector("perf", true, "", "perf execution not implemented"));
  } else {
    data.collector_results.push_back(recordCollector("perf", false, ""));
  }

  if (options.strace) {
    data.collector_results.push_back(
        recordCollector("strace", true, "", "strace execution not implemented"));
  } else {
    data.collector_results.push_back(recordCollector("strace", false, ""));
  }

  if (options.command_str) {
    int status = 0;
    waitpid(static_cast<pid_t>(target_pid), &status, 0);
  }

  data.snapshot = normalizeDiagnostics(data.artifacts, target, data.collector_results);
  nlohmann::json snapshot_json = data.snapshot;
  writeFile(data.artifact_dir + "/normalized.json", snapshot_json.dump(2));
  return data;
}

std::optional<DiagnosticsSnapshot> loadSnapshot(const std::string &input) {
  auto content = readFile(input + "/normalized.json");
  if (content.empty()) {
    return std::nullopt;
  }
  auto json = nlohmann::json::parse(content, nullptr, false);
  if (json.is_discarded()) {
    return std::nullopt;
  }
  return snapshotFromJson(json);
}

std::string analyze(const std::string &output_dir, const std::string &model,
                    DiagnosticsSnapshot &snapshot) {
  OllamaClient client;
  auto result = client.analyze(snapshot, model);
  std::string response = result.response;
  if (!output_dir.empty()) {
    writeFile(output_dir + "/analysis.txt", response);
  }
  return response;
}

} // namespace proccli

int main(int argc, char **argv) {
  std::string error;
  auto options_opt = proccli::parseArgs(argc, argv, error);
  if (!options_opt) {
    if (!error.empty()) {
      std::cerr << error << "\n";
      proccli::printUsage();
      return 1;
    }
    return 0;
  }
  auto options = *options_opt;

  try {
    if (options.command == proccli::CommandType::Collect) {
      auto data = proccli::collect(options);
      std::cout << "Artifacts stored at: " << data.artifact_dir << "\n";
      return 0;
    }

    if (options.command == proccli::CommandType::Analyze) {
      auto snapshot_opt = proccli::loadSnapshot(options.input);
      if (!snapshot_opt) {
        std::cerr << "Unable to load normalized snapshot." << "\n";
        return 1;
      }
      auto snapshot = *snapshot_opt;
      proccli::analyze(options.input, options.model, snapshot);
      std::cout << "Analysis complete." << "\n";
      return 0;
    }

    if (options.command == proccli::CommandType::Report) {
      auto snapshot_opt = proccli::loadSnapshot(options.input);
      if (!snapshot_opt) {
        std::cerr << "Unable to load normalized snapshot." << "\n";
        return 1;
      }
      auto analysis = proccli::readFile(options.input + "/analysis.txt");
      auto report = proccli::renderReport(analysis, *snapshot_opt);
      if (options.format == "json") {
        nlohmann::json snapshot_json = *snapshot_opt;
        report = snapshot_json.dump(2);
      }
      if (!options.output.empty()) {
        proccli::writeFile(options.output, report);
      }
      std::cout << report << "\n";
      return 0;
    }

    auto data = proccli::collect(options);
    auto analysis = proccli::analyze(data.artifact_dir, options.model, data.snapshot);
    auto report = proccli::renderReport(analysis, data.snapshot);
    proccli::writeFile(data.artifact_dir + "/report.txt", report);
    if (!options.output.empty()) {
      proccli::writeFile(options.output + "/report.txt", report);
    }
    std::cout << report << "\n";
  } catch (const std::exception &ex) {
    spdlog::error("Unhandled error: {}", ex.what());
    return 1;
  }
  return 0;
}
