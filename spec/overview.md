# procCli Specification Overview

## Goal
Build a C++ CLI tool that collects system/process diagnostics from `valgrind`, `ps`, `/proc`, `perf`, and `strace`, then uses a **local Dockerized Ollama server** to analyze the data, explain findings, and recommend actions in plain text.

## Key Outcomes
- Consistent capture of process and system signals from each data source.
- Normalized data model so analysis can reason across inputs.
- Clear, human-readable report that includes findings and recommended actions.
- Designed for local-only operation; **no data leaves the host**.
- Predictable artifact storage for collection history and analysis.

## Non-Goals
- A full GUI or web dashboard.
- Real-time continuous monitoring (initial release is batch analysis).
- Automated remediation. The tool provides recommendations only.

## Tech Stack
- Language: **C++17 or later**
- Logging: **spdlog**
- Tests: **googletest**
- Model server: **Ollama** running locally in Docker

## Deliverables
- CLI executable `proccli` (name TBD)
- Modular collectors and analyzers
- Text report output (stdout and/or file)
- Tests for parsing/normalization/reporting
