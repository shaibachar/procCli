# Requirements

## Functional Requirements
1. **Input Capture**
   - `valgrind`: run target command under valgrind and collect leak summary + errors.
   - `ps`: capture process list and relevant fields (`pid`, `ppid`, `cmd`, `rss`, `vsz`, `cpu`, `mem`, `etime`).
   - `/proc`: parse per-process files (`/proc/<pid>/status`, `/proc/<pid>/stat`, `/proc/<pid>/io`) plus system-wide snapshots (`/proc/meminfo`, `/proc/loadavg`).
   - `perf`: use `perf record` + `perf report` when available; attach to target process where possible.
   - `strace`: record syscalls and timing (`-T -tt -f`) for the target process, with output size bounded by the Ollama session size.

2. **Normalization**
   - Map all sources into a common schema (process metadata, resource usage, IO, syscalls, memory issues, CPU hotspots).

3. **Analysis with Ollama**
   - Send normalized data + short context prompt to a local Ollama server in Docker.
   - Receive a structured text response: findings + recommended actions.
   - Provide safe fallback output when the Ollama server is unavailable.

4. **Reporting**
   - Output a single text report to stdout.
   - Optional `--output <path>` flag to save report to file.
   - Include missing-data warnings for any collectors that failed or were disabled.
   - Use a fixed report template with headings for Findings, Recommendations, and Limitations.

5. **CLI UX**
   - Provide subcommands: `collect`, `analyze`, `report` or a single `run` that does all steps.
   - Include `--pid <pid>` or `--command "..."` to define target (mutually exclusive).
   - Offer `--no-<collector>` flags to disable specific sources.
   - When `--pid` and `--command` are both provided, the CLI must error out.
   - For `analyze`/`report`, accept `--input <path>` pointing to a collected artifacts folder.
   - Default artifact storage: if `--output` is not provided, write under a history folder named by timestamp.

## Non-Functional Requirements
- Local-only operation; no external network calls besides local Docker Ollama.
- Deterministic output given the same inputs (except timestamps).
- Reasonable runtime defaults; configurable time limits for perf and strace.
- Clear error messaging and exit codes.
