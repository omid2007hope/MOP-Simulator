# Contribution Guidelines

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

## Review Protocol & Rules
All contributions must adhere to the **OCD Psychopath Code Debugger Protocol**:
1. Zero tolerance for compiler warnings (`-Wall -Wextra`).
2. Clean separation of concern: keep physics equations inside `simulation.cpp`, UI in `main.cpp`, and data parsing in `config_loader.cpp`.
3. All code modifications must pass `mingw32-make test` without broken assertions.
4. Keep simplicity as the foundational principle.
