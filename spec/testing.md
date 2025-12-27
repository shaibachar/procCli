# Testing Strategy

## Unit Tests (googletest)
- Collector parsers (valgrind output, ps output, procfs parsing, strace/perf summaries)
- Normalizer mapping into `DiagnosticsSnapshot`
- Report rendering output
- Golden files for parser outputs and normalized snapshots

## Integration Tests
- Mock tool outputs to simulate a full run.
- Validate Ollama client request payload formatting.
- Mock Ollama server to validate response handling and fallback behavior.

## Logging
- Use spdlog to emit structured logs for debugging.
- Tests can assert on key log messages where appropriate.
