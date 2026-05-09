# Global Rules

## General behavior

- Never delete files without explicit confirmation
- When in doubt, ask before proceeding
- Comment the code you are changing  / writing.
  Comment the code with regard to its domain-specific logic and functionality, not its technical implementation
- For the openspec-new-change skill only create directory and proposal.md.
  Create additional artefacts only if requested.

## Code Style and Conventions

- Make use and be compatible to the C++20 standard

## Testing

- Use Catch2 for unit test
- Maek sure that for every feature unit tests exist

## Calling command line tools

- Prefer using the read tool instead using shell commands for reading files or part of files.
- Prefer non-interactive flags (e.g. `-y`, `--no-input`) in shell commands

## Code Processing

- After every implemented task, make sure that the application builds and all tests are green
- Make sure, that manual corrections via prompting in all openspec phases are correctl represented in the proposal.md, design.md, tasks.md or the spec.md files.
  Update them automatically if needed.

## README.md handling

- Make sure to update the README.md, when..
- ...command line options change
- ...dependencies change

## Application version handling

- Increment the application version in the openspec apply-phase on every change started
- Make use of semantic versioning

## Dependency handling

- Try to reduce dependency, where sensefull
- Update the list of dependencies in the README.md, if dependencies have been change during change execution.
