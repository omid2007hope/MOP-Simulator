# MOP Simulator v2.5 - Comprehensive Code Review

Before moving on to the Version 3.0 roadmap, I have performed a line-by-line review of the entire C++, HTML, and JS codebase. 

Here is a categorized list of all identified bugs, holes, and architectural flaws:

## 1. C++ Core Logic & Data Modeling Flaws

### 1.1 `Target` Material Properties are Hardcoded
- **The Flaw**: The `Target` struct in `simulation.hpp` only stores `name` and `density`. However, the physics engine (specifically the rigid body penetration equation) requires the **Compressive Bearing Strength** (`rt`). 
- **The Impact**: In `simulation.cpp`, the bearing strength is hardcoded to `cons.compressiveStrengthOfUHPC` (200 MPa) for **all targets**. This means whether you strike soft soil or hardened steel armor, the target is treated as Ultra-High Performance Concrete for deceleration!
- **The Fix**: Add `double compressive_strength;` to the `Target` struct and load it from `data/targets.json`.

### 1.2 `main.cpp` Custom Input Logic Bugs
- **The Flaw**: In the interactive CLI mode (Choice 2), there is a severe bug regarding the projectile name:
  ```cpp
  std::string projectileName;
  getline(std::cin, projectileName);
  if (!projectileName.empty())
      projectileName = "Undefined Projectile";
  ```
  First, this uses `!projectileName.empty()` which means if the user *does* type a name, it gets overwritten with "Undefined Projectile". It should be `if (projectileName.empty())`. 
- **The Flaw**: `getline(std::cin, ...)` is called immediately after `std::cin >> choice`. `cin` leaves the newline character `\n` in the buffer, so `getline` reads an empty string instantly and skips the prompt. We need `std::cin.ignore()`.
- **The Flaw**: The custom `projectileName` is never assigned to `mop.name`, so the report will still incorrectly label the custom weapon as "GBU-57 MOP".

### 1.3 Missing Input Validation Edge Cases
- **The Flaw**: In `main.cpp`, `getValidInput<T>` does not check for `EOF` (End of File). If the standard input stream is closed or interrupted (e.g., `Ctrl+Z`), the `while(allowEntry == true)` loop will spin infinitely, causing a 100% CPU lockup and console spam.
- **The Flaw**: The physics engine does not validate `total_mass >= explosive_mass`. A user could enter 500kg total mass and 1000kg explosive mass, which breaks the laws of physics and the simulation.

## 2. WebGL & JavaScript (Frontend) Flaws

### 2.1 Severe Memory Leak in Three.js (Slider Tuning)
- **The Flaw**: In `assets/visualizer_template.html`, the `updateVisuals(data)` function is called every time a slider is dragged. Inside this function, the code does:
  ```javascript
  if (targetBlock) scene.remove(targetBlock);
  // ... creates new THREE.BoxGeometry and THREE.Mesh ...
  scene.add(targetBlock);
  ```
- **The Impact**: In Three.js, removing a mesh from the scene **does not free it from GPU memory**. You must explicitly call `.dispose()` on the geometry and material. Dragging the slider 100 times creates 100 orphaned 3D blocks in VRAM, which will quickly crash the browser window out of memory (OOM).

### 2.2 Cross-Site Scripting (XSS) / Syntax Breaking Injection
- **The Flaw**: When generating `3d_visualizer.html`, C++ injects strings directly into JavaScript:
  ```cpp
  data << "            { name: \"" << r.scenario_name << "\", ...
  ```
- **The Impact**: If a custom scenario name contains a quotation mark (e.g., `Custom "Bunker Buster"`), it will break the JavaScript syntax, and the entire Web UI will fail to load, resulting in a blank screen. The C++ code needs to escape quotes before injecting strings into JS.

## 3. General Architecture & Best Practices

### 3.1 Global Variable Shadowing in `main.cpp`
- **The Flaw**: `double valueEntry;` and `int choice = 2;` are declared globally at the top of `main.cpp`. Then, inside `getValidInput`, `T valueEntry;` is declared again, shadowing the global variable. Global state is completely unnecessary here and should be refactored into local function scope.

### 3.2 File Path Fragility
- **The Flaw**: `ConfigLoader` hardcodes the paths `"data/targets.json"` and `"assets/visualizer_template.html"`. 
- **The Impact**: If the user runs `sim.exe` from inside the `bin/` directory instead of the project root directory, the program will silently fail to find the databases and HTML template, defaulting to the fallback parameters without any visualizer output.

---

### Recommendation for Proceeding
Before jumping into Version 3.0 (Advanced Physics like multi-layer targets), we should lock down these structural holes. Which area would you like to patch first? I recommend fixing the **Target Compressive Strength injection** or the **Three.js memory leak**.
