# AI Code Style & Quality Standards

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

## Core Philosophy
"An idiot admires complexity; a genius admires simplicity."

## C++23 Standards
1. **Simplicity First**: Prefer the smallest correct solution. Avoid over-engineering, unnecessary abstractions, and template metaprogramming bloat.
2. **Explicit Interfaces**: Use explicit types, `std::optional`, and clear method signatures. Avoid opaque pointers or untyped magic numbers.
3. **Memory Safety**: Use pass-by-const-reference (`const T&`) for structures and models. Avoid raw pointer allocations (`new`/`delete`). Use stack allocation or standard containers (`std::vector`).
4. **Formatting**:
   - 4-space indentation.
   - Braces `{}` on the same line for functions and loops where standard in the codebase, or consistent 4-space indentation blocks.
   - Keep comments concise and only explain non-obvious physics, math, or logic.
5. **No Warnings**: All code must compile cleanly under `-Wall -Wextra -O2 -std=c++23` with 0 compiler warnings.
