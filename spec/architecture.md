# Architecture

## High-Level Components
- **CLI Layer**
  - Parses args and orchestrates execution flow.
- **Collectors**
  - `ValgrindCollector`, `PsCollector`, `ProcfsCollector`, `PerfCollector`, `StraceCollector`.
- **Normalizer**
  - Converts raw tool outputs into a common `DiagnosticsSnapshot` model.
- **Schema**
  - Explicit JSON schema for `DiagnosticsSnapshot` with types and required fields (see `spec/schema.md`).
- **Ollama Client**
  - Sends prompt + JSON data to local Ollama via HTTP.
- **Report Renderer**
  - Converts model output into final human-readable text.

## Data Flow
1. CLI determines target and enabled collectors.
2. Collectors run and emit raw artifacts (text, JSON, logs) into an artifact directory.
3. Normalizer parses artifacts into structured data.
4. Ollama client sends a compact prompt + structured data.
5. Renderer produces findings and recommended actions.

## DiagnosticsSnapshot (Conceptual)
- `target`: pid/command
- `system`: loadavg, meminfo
- `processes`: list of process summaries (ps + procfs)
- `valgrind`: errors, leak summary
- `perf`: cpu hotspots, top symbols (if available)
- `strace`: top syscalls, slow syscalls
- `io`: per-process read/write
- `timing`: capture timestamps
- `quality`: per-collector status, errors, and partial-data flags

## Artifact Layout (Conceptual)
- `artifacts/<timestamp>/`
  - `raw/` (tool outputs)
  - `normalized.json` (DiagnosticsSnapshot)
  - `analysis.txt` (model output)
  - `report.txt` (final report)

## Error Handling
- Each collector should fail independently and report partial results.
- Report should include warnings when data sources are missing.
