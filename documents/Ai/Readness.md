# Project Readiness & Coding Guidelines

## 1. Copyright Header

Every source file **must** include the following copyright notice on **Line 1** at the very top:

```cpp
// Copyright (c) 2026 Omid Teimory. All Rights Reserved
```

---

## 2. Include / Import / Require Conventions

Always organize `#include`, `import`, `require`, and similar statements into explicit blocks for **packages/system libraries** and **local files**.

### Grouping & Spacing Rules

1. Place external packages and standard libraries first under the `// packages` comment header.
2. Insert **exactly one empty line** between package imports and file imports.
3. Place local project files under the `// files` comment header.

### Example

```cpp
// packages
#include <cctype>

// files
#include "config_loader.hpp"
```

---

## 3. Indentation & Spacing Rules

### Indentation Guidelines

- **0 Tabs**: Outer function declarations and top-level definitions.
- **1 Tab**: Statements inside the function body (e.g., `if` conditions).
- **2 Tabs**: Statements inside nested blocks (e.g., body of an `if` block).

### Line Break Rules

- **Block Headers**: Always insert a newline (`enter`) at the top of a block body after the opening brace `{`.
- **Block Separators**: Always insert a newline (`enter`) before major statements following a closed block `}`.

---

## 4. Commenting Conventions

Applicable languages: **JavaScript**, **Node.js**, **React**, **C++**, **C**, **C#**.

### Rules & Syntax

- **Special Comments (`// !`)**: Use red special comments to describe what a function does at the top, and mark where the function ends (`// **** Ends Here ****`).
- **Normal Comments (`//`)**: Use normal comments before block conditions (e.g., `if`) to explain **why** the logic exists.

### Code Example

```cpp
// ! What it does -- ! red special comment
void safeCin() {
    // comment why if -- // normal comment
    if (!std::cin) {

        std::cin.clear();
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    // **** Ends Here **** -- ! red special comment
}
```

---

## 5. Function Spacing Rules

### Inter-Function Separation

- Always insert **exactly 4 empty lines** (line spaces) between top-level functions in source code files.

### Example

```cpp
// ! Function 1
void firstFunction() {
    // ...
    // **** Ends Here **** -- ! red special comment
}




// ! Function 2 (4 empty lines above)
void secondFunction() {
    // ...
    // **** Ends Here **** -- ! red special comment
}
```

---

## 6. Console I/O & Print Spacing Rules (`cin`, `cout`, `print`)

### Rules & Formatting

- **Indentation**: Always use **1 Tab** indentation for `std::cin`, `std::cout`, or `print` blocks inside functions.
- **Spacing**: Always insert **2 empty lines** (line spaces) **above** and **below** console print/input blocks.

### Example

```cpp
void displayEula() {


    std::cout << "=================================================================================\n";
    std::cout << "                 C++ IMPACT PHYSICS & PENETRATION SIMULATOR V2.8                 \n";
    std::cout << "=================================================================================\n\n";
    std::cout << "[!] END-USER LICENSE AGREEMENT (EULA) & TERMS OF SERVICE [!]\n";
    std::cout << "WARNING: This software is a high-fidelity physics simulator.\n";
    std::cout << "Usage is strictly restricted to educational and hobbyist purposes.\n\n";
    std::cout << "DISCLAIMER OF WARRANTY: Provided \"AS IS\", without warranty of any kind.\n";
    std::cout << "LIMITATION OF LIABILITY: Author(s) are not liable for any claim or damages.\n";


    // **** Ends Here **** -- ! red special comment
}
```

