# MOP Simulator: Project Vision & Roadmap

## 🌟 Strategic Vision

The MOP (Massive Ordnance Penetrator) Simulator is envisioned to evolve from a basic CLI calculator into a highly modular, interactive, and academically robust physics simulation suite. The ultimate goal is to bridge the gap between complex terminal ballistic mathematics and accessible, real-time 3D visualizations for educational, research, and engineering purposes.

Future iterations will move away from hardcoded variables and rigid logic towards a data-driven architecture, enabling complex multi-layered target simulations, thermodynamic modeling, and seamless integration with modern web and scripting ecosystems.

---

## 🛠️ Current Codebase Review & Bug Tracker

During the latest comprehensive line-by-line code review, the following issues and technical debts were identified:

### 🐛 Active Bugs

- [x] **Hardcoded JS Physics Bug**: In `simulation.cpp` (Line 528), the generated WebGL JavaScript recalculates `hydroDepth` using a hardcoded casing density of `7800.0`. If a custom projectile is used with a different density (e.g., Tungsten at 19300.0), the HTML visualizer's parametric sliders will calculate and render the wrong depth.
- [x] **Typo in Default Scenarios**: In `main.cpp` (Line 122), the 4th preset scenario is named "Subsonic Operational" with an altitude of `15.0` ft. Based on preceding entries (50,000, 25,000, 18,000), this is likely a typo for `15000.0` ft.
- [x] **Interactive Input Loop Failure**: In `main.cpp`, if the user enters invalid input during the interactive prompt (e.g., a string instead of a number), `clearCinBuffer()` is called, and the program silently defaults to the previous value instead of re-prompting the user.
- [x] **Scenario Count Constraint Mismatch**: In `main.cpp` (Line 80), the prompt limits custom impact velocities to `[1 to 5]`, but the actual validation logic on Line 82 accepts up to `10`.

### 🏗️ Technical Debt & Areas of Improvement

- [ ] **Decouple HTML Generation**: Embedding 400+ lines of raw HTML/JS in `simulation.cpp` as a raw string literal makes UI maintenance difficult. **Action:** Move the HTML template into an external asset file (e.g., `assets/visualizer_template.html`) and load/inject data at runtime.
- [ ] **Parameterize Magic Numbers**: Constants like drag coefficient (`cd = 1.2`), target compressive strength (`rt = 100.0e6`), and the speed of sound (`SPEED_OF_SOUND = 343.0`) are hardcoded. **Action:** Extract these into a `PhysicsConstants` structure or configuration file.
- [ ] **Standardize Math Constants**: Replace manual definitions of Pi (`3.14159...`) with C++ standard library equivalents (e.g., `std::numbers::pi` in C++20 or `M_PI`).

---

## 🚀 Version 2.0 Roadmap: Modularity & Data-Driven Design

_Focus: Refactoring the core architecture to support dynamic inputs, cleaner code separation, and flawless 3D WebGL synchronization._

- [ ] **External Asset Loading System**: Implement a file-reading utility to load the WebGL HTML/JS template dynamically, eliminating the massive string literal in the C++ source.
- [ ] **Data-Driven Configuration**: Introduce a JSON or YAML parser (e.g., `nlohmann/json`) to load external databases of:
  - **Targets**: Soil, Granite, High-Quality Concrete, Steel Armor.
  - **Projectiles**: GBU-57 MOP, BLU-109, Custom Tungsten Rods.
- [ ] **Web UI Physics Synchronization**: Fix the hardcoded JS physics bug by injecting the dynamic `casing_density` and `target_density` properties into the HTML payload.
- [ ] **Robust CLI Interface**: Refactor `main.cpp` interactive prompts to use robust `while` loops that enforce strict validation and re-prompt on invalid user input.

---

## 🌌 Version 3.0 Roadmap: Advanced Physics & Modern Tooling

_Focus: Pushing the boundaries of the physics engine and introducing professional integrations._

- [ ] **Multi-Layered Target Simulation**: Upgrade the physics engine to calculate penetration through complex stratifications (e.g., 10m earth -> 5m reinforced concrete -> 1m steel vault).
- [ ] **Deformation & Thermodynamics**: Introduce advanced mechanics simulating projectile mushrooming, mass loss via hydrodynamic erosion, and heat generation during hypervelocity impacts.
- [ ] **Embedded Web Server GUI**: Replace static HTML generation with an embedded lightweight C++ web server (e.g., `cpp-httplib`). This will stream live simulation telemetry and 3D positional data to the browser in real-time.
- [ ] **Python Bindings (pybind11)**: Create a Python API wrapper for the simulation core, empowering data scientists and engineers to run massive batch impact simulations and parameter sweeps directly from Jupyter Notebooks.
