# procCli

`proccli` is a C++17 command-line tool that collects process/system diagnostics from standard Linux
tooling (`ps`, `/proc`, `perf`, `strace`, `valgrind`) and uses a local Dockerized Ollama server to
analyze the data and generate a human-readable report. The tool is designed to run locally only;
no data leaves the host.

## Features

- Collect diagnostics from multiple sources and normalize them into a single JSON snapshot.
- Analyze normalized data using a local Ollama model with safe fallback output when Ollama is unavailable.
- Render reports in text or JSON format.
- Modular collectors with per-collector status reporting.

## Requirements

- C++17 toolchain
- CMake 3.16+
- Git (to fetch dependencies via CMake FetchContent)
- Optional: local Ollama server running in Docker at `http://localhost:11434`

## Build

```bash
cmake -S . -B build
cmake --build build
```

The CLI binary will be available at `build/proccli`.

## Usage

```bash
# Run collection + analysis + report (default)
./build/proccli run --command "./app --arg"

# Collect only
./build/proccli collect --pid 1234 --output artifacts/run-1

# Analyze an existing artifacts folder
./build/proccli analyze --input artifacts/run-1

# Render a report from an existing artifacts folder
./build/proccli report --input artifacts/run-1 --format text
```

### Key Options

- `--pid <pid>`: target a running process (mutually exclusive with `--command`).
- `--command "<cmd>"`: run and monitor a command.
- `--output <path>`: write report (or artifacts for `collect`) to a path.
- `--input <path>`: use an existing artifacts folder for `analyze`/`report`.
- `--format text|json`: output report format (text default).
- `--no-<collector>`: disable a collector (`valgrind`, `ps`, `proc`, `perf`, `strace`).
- `--model <name>`: Ollama model name (defaults to `llama3`).

## Artifacts Layout

By default, artifacts are stored under a timestamped folder in `artifacts/`:

```
artifacts/<timestamp>/
  raw/
  normalized.json
  analysis.txt
  report.txt
```

## Contributing

1. Fork the repository and create a feature branch.
2. Make changes with clear, focused commits.
3. Run tests locally before submitting:

   ```bash
   cmake -S . -B build
   cmake --build build
   ctest --test-dir build
   ```

4. Open a pull request with a summary of changes and testing notes.
