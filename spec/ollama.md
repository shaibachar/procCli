# Ollama Integration Specification

## Deployment Assumption
- Ollama runs locally via Docker, exposed on `http://localhost:11434`.
- Models are pre-pulled locally (e.g., `llama3`, `mistral`) with a configurable default.

## Request Format
- Use Ollama HTTP API (`/api/generate` or `/api/chat`).
- Provide:
  - System prompt describing the role: “diagnostic analyst”.
  - User payload: a compact JSON snapshot + brief text instructions.
  - Token/response length limits aligned with model configuration.

## Prompt Template (Conceptual)
- System: “You analyze Linux diagnostics, explain findings, and recommend actions.”
- User:
  - “Given this JSON snapshot, list key findings and recommended actions.”
  - Attach `DiagnosticsSnapshot` as JSON.

## Response Expectations
- Plain text sections:
  - Findings (bullet list)
  - Recommendations (bullet list)
  - Unknowns/Limitations (if missing data)

## Safety/Privacy
- All data stays local.
- No external network calls.
- If Ollama is unavailable, emit a report noting analysis could not be completed.
