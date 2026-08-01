# C++ Input Validation: `while(true)` and `break`

The `while(true)` and `break` pattern is a standard C++ idiom used for **Input Validation**. It ensures that a program will continuously prompt the user until a valid value is entered before allowing execution to proceed.

---

## Code Example

Below is the implementation used in [`src/main.cpp`](file:///h:/Code/MyOwn/Main/Best/MOP%20Simulator/src/main.cpp#L129-L135) for checking explosive mass:

```cpp
while (true) {
    mop.explosive_mass = getValidInput<double>("Enter Explosive Mass (kg): ", true);

    if (mop.explosive_mass <= mop.total_mass)
        break; // Escapes the loop if the input is valid!

    std::cout << "Error: Explosive mass cannot exceed total mass!\n";
}
```

---

## How It Works

### 1. `while(true)` (The Infinite Loop)
`while(true)` creates a loop that runs continuously because its condition evaluates to `true` on every pass. 

> [!NOTE]
> An infinite loop is ideal for user input because the program cannot predict how many attempts the user will need before providing a valid value.

### 2. `break` (The Escape Hatch)
The `break` statement immediately terminates the loop and jumps to the first line of code following the loop.

- **Loop Trapping:** The program stays trapped inside the loop until a valid input satisfies the `if` condition.
- **Escape:** Once valid input is provided (`explosive_mass <= total_mass`), `break` triggers, allowing execution to proceed.

---

## Comparison: `while(true)` vs. Flag Variable

### Approach 1: `while(true)` with `break` (Preferred)
- Direct and readable top-to-bottom flow (*"Keep trying until valid, then stop"*).
- No need to introduce extra status flags or dummy variables.

### Approach 2: Using a Boolean Flag
```cpp
bool isValid = false;
while (!isValid) {
    mop.explosive_mass = getValidInput<double>("Enter Explosive Mass (kg): ", true);
    if (mop.explosive_mass <= mop.total_mass) {
        isValid = true; // Loop finishes after completing iteration
    } else {
        std::cout << "Error: Explosive mass cannot exceed total mass!\n";
    }
}
```

---

## Summary / Key Takeaway

> [!TIP]
> The program repeatedly prompts the user in a loop. Only when a valid entry is passed does `break` execute, allowing the execution flow to continue to the next lines of code.
