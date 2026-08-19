# 5.2 Code Style & Formatting Standards

// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

---

## 🏛️ Core Engineering Philosophy

> *"An idiot admires complexity; a genius admires simplicity."* — Terry A. Davis

The **MOP Simulator** codebase prioritizes simplicity, physical clarity, readability, and deterministic behavior over abstract over-engineering.

---

## ⚙️ C++23 Standards & Best Practices

1. **Simplicity First**:
   - Prefer the smallest correct implementation. Avoid unnecessary inheritance trees, dynamic dispatch overhead, and complex template metaprogramming bloat.

2. **Memory Safety & Lifetime Management**:
   - Pass complex structs and vectors by constant reference (`const Projectile&`, `const Target&`).
   - Avoid raw pointer allocations (`new` / `delete`). Use standard STL containers (`std::vector`, `std::string`) or value types on the stack.
   - Use `const` correctness aggressively for immutable configuration fields.

3. **Explicit Type Signatures**:
   - Use explicit scalar types (`double`, `int`, `bool`) and `std::optional` where values may be absent.
   - Avoid untyped magic numbers; define physical constants inside `PhysicsConstants` or `default.hpp`.

4. **Compiler Warning Enforcement**:
   - All code must compile cleanly under `-Wall -Wextra -O2 -std=c++23` with **zero compiler warnings**.

5. **Clang-Format Style Rules (`.clang-format`)**:
   - Indentation: 4 spaces (or tabs matching existing files).
   - Column Limit: 100–120 characters.
   - Braces: Attach opening brace to control statements (`if`, `for`, `while`, `struct`).

---

## 🌐 JavaScript & Node.js Standards

1. **Architectural Separation of Concerns**:
   - **Router Layer (`router/`)**: Endpoint routing and middleware mapping only.
   - **Controller Layer (`controller/`)**: Request parsing, parameter validation, and HTTP response formatting.
   - **Service Layer (`service/`)**: Business logic, IPC spawning, statistical analysis, and orchestration.
   - **Data Layer (`model/`)**: Mongoose schema declarations and indices.

2. **Asynchronous Stream Discipline**:
   - Use `async` / `await` and `for await` stream generators. Never use raw nested callbacks.
   - Enforce stream backpressure and batch database insertions (1,000-frame chunks).

3. **Prettier Formatting (`.prettierrc`)**:
   - Single quotes, trailing commas where valid, 2-space or 4-space consistent indentation.

---

## 🔍 Surgical Fix Directives

* **Preserve Established Patterns**: When modifying existing code, maintain the existing style, naming conventions, and structural idioms.
* **Full Context Lineage**: When updating physical equations or parameters, audit every call site and telemetry export field to guarantee zero unintended side effects.

---

## 🧭 Navigation

* [Back to 5. Testing & Quality Assurance](05-testing-and-quality-assurance.md)
* [Proceed to 6. Reference Materials & Research Papers](06-reference-materials-and-research-papers.md)
* [Explore 1.2 Project Roadmap & Contribution Guide](01-02-roadmap-and-contribution.md)
