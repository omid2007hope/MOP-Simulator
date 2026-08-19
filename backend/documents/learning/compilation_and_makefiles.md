# How C++ Compiles: Preprocessing, Compiling, Linking & Makefiles

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

---

## 1. The 4 Stages of C++ Compilation

Unlike interpreted languages like Python or JavaScript that execute line-by-line at runtime, C++ is compiled directly into **machine code instructions** executed by the CPU.

```
+--------------------+      +------------------+      +-------------------+      +------------------+
| C++ Source Files   |  ->  |  Preprocessed    |  ->  | Object Files      |  ->  | Binary Executable|
| (.cpp & .hpp)      |      |  Source Code     |      | (.o / .obj)       |      | (.exe)           |
+--------------------+      +------------------+      +-------------------+      +------------------+
                             (Stage 1: Preproc)        (Stage 2 & 3: Comp)       (Stage 4: Linker)
```

### Stage 1: Preprocessing
The preprocessor handles directives starting with `#`:
- `#include "simulation.hpp"`: Copies the contents of header files into the source file.
- `#ifndef` / `#define` / `#endif`: Header guards that prevent multiple inclusions.

### Stage 2: Compilation
The compiler checks C++ syntax, type safety, and converts high-level C++ statements into Assembly code.

### Stage 3: Assembly
The assembler translates assembly into binary machine instructions, creating **Object Files** (`build/main.o`, `build/simulation.o`).

### Stage 4: Linking
The **Linker** stitches all object files (`.o`) together with external C++ libraries to generate the final standalone executable binary (`bin/mop_sim.exe`).

---

## 2. Understanding compiler flags (`g++`)

Look at the `Makefile` in the project root:

```makefile
CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O2 -Iinclude
```

| Flag | Meaning |
| :--- | :--- |
| `-std=c++23` | Enforces the modern C++23 standard. |
| `-Wall` | Enables **All** common compiler warnings. |
| `-Wextra` | Enables **Extra** warning checks for code safety. |
| `-O2` | Enables **Level 2 Compiler Optimization** (vectorization, loop unrolling, register allocation). |
| `-Iinclude` | Tells compiler to look inside the `include/` folder for `#include` header headers. |

---

## 3. How `Makefile` Automates Building

Instead of typing long compilation commands manually every time you change code, a `Makefile` specifies targets and dependencies:

```makefile
# Target: Build binary from object files
bin/mop_sim.exe: build/main.o build/simulation.o build/config_loader.o
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule: Compile main.cpp to build/main.o
build/main.o: src/main.cpp include/simulation.hpp include/config_loader.hpp include/default.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
```

### Key Makefile Automatic Variables:
- `$@`: The target filename being generated (e.g. `bin/mop_sim.exe`).
- `$<`: The first dependency file (e.g. `src/main.cpp`).
- `$^`: All dependency files listed.

---

## 4. Running the Build Command

To compile the entire project, simply run in terminal:

```bash
mingw32-make
```

To build and run the test suite:

```bash
mingw32-make test
```

To clean up compiled binaries:

```bash
mingw32-make clean
```
