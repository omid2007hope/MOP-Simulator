# C++ Memory Management: Pass by Value vs. Pass by Reference

The difference between these two parameter-passing methods comes down to how C++ moves data around in memory.

---

## 1. Passing by Value (`double v`, `double rho_p`)
> **Analogy:** Handing a coworker a **photocopy** of your document.

When a function takes a simple type like `double v`, C++ makes a brand-new copy of the number in memory for the function to use.

* **Why use it here?** `double`, `int`, and `bool` are incredibly tiny data types (just 8 bytes). Copying them is lightning fast for the computer, making it the standard way to pass simple scalar values.

---

## 2. Passing by Constant Reference (`const Aircraft& bomber`)
> **Analogy:** Handing a coworker a **read-only digital link** to your document.

`Aircraft`, `ImpactScenario`, and `AtmosphereState` are `structs`—custom packages that hold multiple variables inside them. If we just wrote `Aircraft bomber` (without the `const &`), C++ would have to copy the entire package and all its contents every time the function is called. For large structures, this wastes memory and slows down execution.

Instead, we use **`const &`**:

* **`&` (Reference):** Tells C++ *"Don't copy the data. Just give the function a shortcut pointing directly to the memory address of the original object."*
* **`const` (Constant):** A safety guarantee. Tells C++ *"This function will only READ the data—it is strictly forbidden from modifying or deleting it."*

---

## Summary: The C++ Golden Rule

* **For simple, small numbers** (`double`, `int`, `bool`): Pass them normally (**Pass by Value**). It is fast and simple.
* **For custom structs, classes, or large data** (`Aircraft`, `ImpactScenario`, `std::string`): Always pass them as **`const Type&`** (**Pass by Const Reference**) to prevent making expensive copies in memory.
