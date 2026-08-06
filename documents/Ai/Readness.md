# Project Readiness & Coding Guidelines

Universal coding standards, file structure conventions, and formatting guidelines for **JavaScript**, **Node.js**, **React**, **C++**, **C**, and **C#**.

---

## Table of Contents

1. [Copyright Header](#1-copyright-header)
2. [Include / Import / Require Conventions](#2-include--import--require-conventions)
3. [Indentation & Spacing Rules](#3-indentation--spacing-rules)
4. [Commenting Conventions](#4-commenting-conventions)
5. [Function Spacing Rules](#5-function-spacing-rules)
6. [Console I/O & Print Spacing Rules](#6-console-io--print-spacing-rules-cin-cout-print)
7. [Comprehensive Master Example](#7-comprehensive-master-example)
8. [AI Readiness & Code Review Checklist](#8-ai-readiness--code-review-checklist)

---

## 1. Copyright Header

Every single source file **must** include the following exact copyright notice on **Line 1** at the very top:

```cpp
// Copyright (c) 2026 Omid Teimory. All Rights Reserved
```

---

## 2. Include / Import / Require Conventions

Always organize `#include`, `import`, `require`, and similar header statements into two explicit, structured sections: **packages/system libraries** and **local project files**.

### Grouping & Spacing Rules

1. Place external packages and standard libraries first under the `// packages` comment header.
2. Insert **exactly one empty line** between package imports and file imports.
3. Place local project files under the `// files` comment header.

### Example

```cpp
// packages
#include <cctype>
#include <iostream>

// files
#include "config_loader.hpp"
#include "simulation.hpp"
```

---

## 3. Indentation & Spacing Rules

### Indentation Guidelines

- **0 Tabs**: Template headers, top-level function declarations, and top-level closing braces.
- **1 Tab**: Direct statements inside function body (`T valueEntry;`, `while` statement).
- **2 Tabs**: Statements inside level 1 nested loops (`while`).
- **3 Tabs**: Statements inside level 2 nested blocks (`if (std::cin >> valueEntry)`).
- **4 Tabs**: Statements inside level 3 nested blocks (`if (valueEntry > 0 ...)`).
- **5 Tabs**: Core statements inside deep nested blocks (`safeCin();`, `return valueEntry;`).

> **Rule:** Every nested block level increases indentation by **1 Tab (+1 level deeper)**.

### Nested Indentation Example

```cpp
template <typename T>
T getValidInput(const std::string& prompt, bool allowZero = false) {
    T valueEntry;

    while (true) {

        std::cout << prompt;

        if (std::cin >> valueEntry) {

            if (valueEntry > 0 || (allowZero && valueEntry == 0)) {

                safeCin();
                return valueEntry;
            }
        }
    }
}
```


---

## 4. Commenting Conventions

Applicable languages: **JavaScript**, **Node.js**, **React**, **C++**, **C**, **C#**.

### Rules & Syntax

- **Special Comments (`// !`)**: Use red special comments at the top of a function to state **what the function does**, and at the end of the function body to mark its exit (`// **** Ends Here ****`).
- **Normal Comments (`//`)**: Use standard comments before conditional blocks (e.g., `if`) to explain **why** the logic exists.

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

---

## 7. Comprehensive Master Example

This complete example combines all 6 coding rules into a single production-ready template:

```cpp
// Copyright (c) 2026 Omid Teimory. All Rights Reserved

// packages
#include <cctype>
#include <iostream>
#include <limits>

// files
#include "config_loader.hpp"
#include "simulation.hpp"


// ! Ensures input stream safety by clearing error flags and ignoring invalid characters
void safeCin() {
    // comment why if -- handle broken cin state before clearing buffer
    if (!std::cin) {

        std::cin.clear();
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    // **** Ends Here **** -- ! red special comment
}




// ! Displays software license agreement and legal disclaimers to user
void displayEula() {


    std::cout << "=================================================================================\n";
    std::cout << "                 C++ IMPACT PHYSICS & PENETRATION SIMULATOR V2.8                 \n";
    std::cout << "=================================================================================\n\n";
    std::cout << "[!] END-USER LICENSE AGREEMENT (EULA) & TERMS OF SERVICE [!]\n";
    std::cout << "WARNING: This software is a high-fidelity physics simulator.\n";
    std::cout << "Usage is strictly restricted to educational and hobbyist purposes.\n\n";


    // **** Ends Here **** -- ! red special comment
}
```

---

## 8. AI Readiness & Code Review Checklist

Before committing code or submitting pull requests, verify that all guidelines are satisfied:

- [ ] **Line 1 Copyright**: Is `// Copyright (c) 2026 Omid Teimory. All Rights Reserved` on Line 1?
- [ ] **Imports / Includes**: Are imports split into `// packages` and `// files` with 1 empty line in between?
- [ ] **Tab Indentation**: 0 tabs for functions, 1 tab inside functions, 2 tabs inside `if` block bodies?
- [ ] **Block Line Breaks**: Is there an `enter` after opening brace `{` and before statements after `}`?
- [ ] **Special Comments**: Does every function start with a `// !` description and end with `// **** Ends Here ****`?
- [ ] **Function Separation**: Are there **4 empty lines** between consecutive top-level functions?
- [ ] **Console Print Spacing**: Are there **2 empty lines** above and below `cin` / `cout` / `print` blocks?

