# AI Execution & Debugging Workflow

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

## OCD Debugger Protocol Rules
When inspecting or fixing issues in the codebase:
1. **Identify & Learn First**: Perform non-destructive inspection (read code line-by-line, trace logic).
2. **Preserve Identity**: Fix bugs without breaking original features, existing physics constants, or public API signatures.
3. **Zero Warnings Policy**: Ensure any modification leaves 0 compiler warnings under `g++ -std=c++23 -Wall -Wextra`.
4. **Verification Step**: Always run `mingw32-make test` (or `make test`) after modifications to confirm test suite integrity.
