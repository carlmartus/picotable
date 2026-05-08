# Agent Guidance for Pico Table Development

This document guides LLM agents contributing to the pico table header library.

## What to Update When Making Changes

When adding, changing, or removing a feature:

1. **Test cases in `tests/`** - Add or update test cases to cover new functionality, edge cases, and regressions.

2. **`@section agents` in `picotable.h`** - Update the agent guidance doxygen section with patterns and examples for end users' agents. This section teaches autonomous agents how to use the library correctly.

3. **Naming conventions** - Function names follow `Class_functionName` (camelCase after underscore).

4. **README documentation** - Changes to the API must be reflected in the API section in the README.

## Make Directives

The following make directives should pass:
- `make test` - Runs all test cases
- `make samples` - Builds all sample programs
- `make all` - Build everything and runs all test cases

