# Global Rules

## General behaviour

- Never delete files without explicit confirmation
- When in doubt, ask before proceeding
- Comment the code you are changing  / writing. Comment the code with regard to its domain-specific logic and functionality, not its technical implementation

## Code Style and Conventions

- Make use and be compatible to the C++20 standard

## Testing

- Use Catch2 for unit test

## Calling command line tools

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

## Dependecy handling

- Try to reduce dependency, where sensefull
- Update the list of dependencies in the README.md, if dependencies have been change during change execution.
