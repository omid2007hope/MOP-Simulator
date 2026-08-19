# Function Templates & Generic Programming in C++

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

---

## 1. What is a C++ Function Template?

Suppose you need to read and validate user numerical inputs for `int` values (like drop counts: 1 to 5) AND `double` values (like drop altitude: 50,000.0 ft, velocity: 340.5 m/s, or density).

Without templates, you would have to write two duplicate functions:

```cpp
int getValidIntInput(const std::string& prompt);
double getValidDoubleInput(const std::string& prompt);
```

This violates the DRY (**Don't Repeat Yourself**) principle!

C++ **Function Templates** allow you to write a single generic function definition where the type (`T`) is determined at compile time!

---

## 2. Template Implementation in `main.cpp`

Look at [`src/main.cpp`](file:///h:/Code/MyOwn/Main/Best/MOP%20Simulator/src/main.cpp#L24-L43):

```cpp
template <typename T>
T getValidInput(const std::string& prompt, bool allowZero = false)
{
    T valueEntry;
    while (true) {
        std::cout << prompt;
        if (std::cin >> valueEntry) {
            if (valueEntry > 0 || (allowZero && valueEntry == 0)) {
                safeCin();
                return valueEntry;
            }
        } else if (std::cin.eof()) {
            std::cerr << "\n[!] EOF encountered. Exiting safely to prevent infinite loop.\n";
            exit(1);
        }
        
        safeCin();
        std::cout << "Invalid Entry, please try again!\n";
    }
}
```

---

## 3. How the Compiler Instantiates Templates

When you call `getValidInput<double>(...)` or `getValidInput<int>(...)`:

```cpp
// 1. Instantiates a double version at compile time
double alt = getValidInput<double>("Enter Altitude (ft): ", true);

// 2. Instantiates an int version at compile time
int drops = getValidInput<int>("Enter count: ", false);
```

The C++ compiler generates the exact type-safe machine code for `double` and `int` automatically without any runtime performance penalty.

---

## 4. Key Takeaways for Beginners

1. **Syntax:** Use `template <typename T>` before your function header.
2. **Type Parameter `T`:** Acts as a wildcard placeholder for any type (`int`, `double`, `float`, `size_t`).
3. **Compile-Time Safety:** If you pass incompatible input, the C++ compiler catches it before the program even runs!
