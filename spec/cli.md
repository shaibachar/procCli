# CLI Specification

## Command
`proccli [command] [options]`

## Commands
- `run`: collect + analyze + report (default)
- `collect`: collect raw diagnostics only
- `analyze`: analyze existing collected data
- `report`: render report from analysis output

## Core Options
- `--pid <pid>`: target existing process
- `--command "<cmd>"`: run and monitor a command
- `--output <path>`: write report to file
- `--input <path>`: path to previously collected artifacts for `analyze`/`report`
- `--format text|json`: output format (text default)

## Collector Control
- `--no-valgrind`
- `--no-ps`
- `--no-proc`
- `--no-perf`
- `--no-strace`

## Performance/Safety
- `--strace-timeout <sec>`
- `--perf-duration <sec>`
- `--valgrind-tool <memcheck|massif|...>`
 - `--model <name>`: Ollama model (defaults to configured model)

## Validation
- `--pid` and `--command` are mutually exclusive. If both are provided, exit with an error.
- If `--output` is not provided, results are stored under a timestamped history folder.
- `analyze`/`report` require `--input` pointing to a collected artifacts folder.

## Examples
- `proccli run --command "./app --arg"`
- `proccli run --pid 1234 --no-perf`
- `proccli collect --pid 5678 --output artifacts/`
