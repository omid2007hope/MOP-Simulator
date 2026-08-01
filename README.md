# MOP - Massive Ordnance Penetrator Bomb Simulator

A C++ physics and penetration mechanics simulator for high-mass earth-penetrating ordnance like the **GBU-57 Massive Ordnance Penetrator (MOP)**. This project models terminal ballistic impact physics, structural casing integrity, and hydrodynamic soil/concrete penetration limits, featuring both a terminal ASCII cross-section renderer and a dynamically generated **Three.js WebGL 3D interactive visualizer**.

---

## 🛠️ How to Build and Run

### Prerequisites
- Any modern C++ compiler supporting **C++20** or later (`g++`, `clang++`, or MSVC `cl`).
- Optional: `make` / `mingw32-make` or `cmake`.

### Compilation

Using **Make / MinGW Make**:
```bash
make
# or on Windows with MinGW:
mingw32-make
```

Using **GCC / MinGW** directly:
```bash
g++ -std=c++20 -O2 -Iinclude src/main.cpp src/simulation.cpp src/config_loader.cpp -o bin/sim.exe
```

Using **CMake**:
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Running the Simulator

```bash
./bin/sim.exe
```

1. **Select Mode**:
   - `[1]` Run Standard GBU-57 MOP Presets (Mach ~0.9 to Mach ~1.5).
   - `[2]` Interactive Custom Parameter Input.
   - `[3]` Orbital Kinetic Strike Preset ("Rods from God" Tungsten Penetrators).
2. View the console summary report and ASCII cross-sections.
3. The simulator loads `assets/visualizer_template.html`, injects dynamic simulation data via template placeholders, and outputs `3d_visualizer.html` in the root directory.

> **Note:** Projectile and target definitions are loaded from JSON databases in the `data/` directory. If a JSON file is missing or a named entry is not found, the simulator falls back to hardcoded defaults.

#### 🌐 Viewing `3d_visualizer.html`
You can view the 3D scene simply by double-clicking the generated `3d_visualizer.html` file to open it directly in your web browser. No local web server is required.

### Running Unit Tests

To run the automated physics and penetration regime unit tests:
```bash
make test
# or on Windows with MinGW:
mingw32-make test
```

Or with CMake:
```bash
cd build
ctest --output-on-failure
```

---

## 🌟 Key Features

- **Physics-Based Terminal Ballistics**: Evaluates kinetic energy ($E_k$), dynamic impact pressure ($P_{dyn}$), and fluid penetration limits.
- **Alekseevskii-Tate Hydrodynamic Model**: Calculates maximum hydrodynamic penetration depth ($P = L \cdot \sqrt{\rho_p / \rho_t}$) in target media (e.g., hardened concrete).
- **Forrestal Rigid-Body Penetration Model**: Calculates deep underground penetration depth using an ogive-nose CRH-based drag coefficient and a work-energy deceleration model against UHPC compressive strength.
- **Casing Failure & Regimes**: Distinguishes between **Rigid Body Penetration** (deep underground drilling), **Rigid Body Shock Failure** (casing intact but explosive payload damaged by shock), and **Hydrodynamic / Hypervelocity Casing Failure** (surface crushing/detonation).
- **Kinetic Rod Detection**: Automatically identifies zero-explosive / zero-yield projectiles as pure kinetic energy weapons and applies the appropriate hydrodynamic erosion physics regime.
- **Shock Damage Probability**: Calculates the probability of explosive charge failure from impact shock as a function of pressure ratio.
- **Data-Driven Configuration**: Projectile specs and target materials are loaded from external JSON databases (`data/projectiles.json`, `data/targets.json`) using [nlohmann/json](https://github.com/nlohmann/json), with safe hardcoded fallbacks.
- **Dual Visualizer**:
  - **Terminal ASCII 3D Cross-Sections**: Instant visual output in the command line for each scenario (Kinetic Rod, Surface Detonation, and Deep Rigid Penetration modes).
  - **Interactive 3D WebGL HTML Generation**: Generates `3d_visualizer.html` from an external template (`assets/visualizer_template.html`) with Three.js, camera controls, orbit navigation, and HUD metrics.
- **Robust Interactive CLI**: Validated input loops with `getValidInput<T>()` that enforce strict numeric validation and re-prompt on invalid entries.

---

## 🌐 Web Application (Standalone)

The simulator is also available as a **self-contained web application** — no compilation or build step needed.

### Quick Start

1. Open `index.html` in any modern web browser (Chrome, Firefox, Edge, Safari).
2. Select a simulation mode:
   - **GBU-57 MOP Standard** — preset altitude drop scenarios
   - **Custom Parameters** — define your own projectile and target
   - **Orbital Kinetic Strike** — "Rods from God" tungsten penetrators
3. Click **Launch Simulation** → interactive 3D WebGL visualization + data HUD
4. Use parametric sliders for live re-simulation, switch camera views, and toggle the results table.

### Deploy to a Domain

The `index.html` is fully self-contained and can be deployed to any static hosting provider:

**GitHub Pages** (free):
```bash
# Push to a gh-pages branch or enable Pages on main
git add index.html
git commit -m "Add web simulator"
git push
# Then enable GitHub Pages in Settings → Pages → Deploy from branch
```

**Netlify / Vercel**: Drag and drop the project folder, or connect your GitHub repo.

---

## 📐 Physics & Engineering Models

### 1. Kinetic Energy
$$E_k = \frac{1}{2} m v^2$$

### 2. Dynamic Impact Pressure
$$P_{dyn} = \frac{1}{2} \rho_t v^2$$

Where $\rho_t$ is the density of the target material (e.g., $2500 \text{ kg/m}^3$ for hardened concrete).

### 3. Casing Structural Failure Criterion
If $P_{dyn} > \sigma_y$ (where $\sigma_y$ is the yield strength of the casing, e.g. $2.0 \text{ GPa}$ for Eglin steel):
- Casing crushes/shatters upon impact.
- Causes surface/near-surface premature detonation.
- Hydrodynamic limit caps penetration.

If $P_{dyn} \le \sigma_y$:
- Casing remains intact and rigid like a drill bit.
- Shock damage probability is computed: $P_{shock} = \min\left(100,\ \left(\frac{P_{dyn}}{\sigma_y}\right)^{1.5} \times 85\right)$
- If $P_{shock} < 50\%$: explosive survives → smart-fuze triggers deep underground.
- If $P_{shock} \ge 50\%$: explosive fails from shock → premature detonation underground.

### 4. Alekseevskii-Tate Hydrodynamic Penetration Limit
$$P = L \cdot \sqrt{\frac{\rho_p}{\rho_t}}$$
Where:
- $L$ = Projectile length ($\text{meters}$)
- $\rho_p$ = Casing density ($\text{kg/m}^3$)
- $\rho_t$ = Target density ($\text{kg/m}^3$)

### 5. Forrestal Rigid-Body Penetration Model
$$D = \frac{m}{2 A \rho_t N} \cdot \ln\left(1 + \frac{\rho_t N v^2}{2 \sigma_c}\right)$$
Where:
- $m$ = Total projectile mass ($\text{kg}$)
- $A$ = Cross-sectional area of the projectile ($\text{m}^2$)
- $N$ = Ogive nose drag coefficient derived from the Caliber-Radius-Head (CRH): $N = \frac{8 \cdot CRH - 1}{24 \cdot CRH^2}$
- $\sigma_c$ = Compressive strength of UHPC ($200 \text{ MPa}$)

---

## 📁 Repository Structure

```text
MOP Simulator/
├── index.html                    # Standalone web application (UI + Sim + 3D Visualizer)
├── assets/
│   └── visualizer_template.html  # External HTML/JS template for 3D visualizer generation
├── data/
│   ├── projectiles.json          # Projectile database (GBU-57, BLU-109, Tungsten Rod)
│   └── targets.json              # Target material database (Soil, Granite, Concrete, Steel)
├── include/
│   ├── simulation.hpp            # Public API: PhysicsConstants, data structs, ImpactSimulator class
│   ├── config_loader.hpp         # ConfigLoader class for JSON database loading
│   └── nlohmann/                 # nlohmann/json header-only library (vendored)
├── src/
│   ├── main.cpp                  # CLI entry point, mode selection, validated input loops
│   ├── simulation.cpp            # Core physics engine, ASCII renderer, HTML template generator
│   └── config_loader.cpp         # JSON file parsing and projectile/target lookup
├── tests/
│   └── test_simulation.cpp       # Unit tests for ballistics, failure regimes, and kinetic rods
├── bin/                          # Output directory for compiled executables
├── build/                        # Intermediate compilation object files
├── Makefile                      # Standard build script (GCC/Clang/MinGW, C++20)
├── CMakeLists.txt                # CMake build configuration (project V2.5.0)
├── .clang-format                 # Code formatting configuration
├── LICENSE                       # GNU Affero General Public License v3.0
├── README.md                     # Project documentation (this file)
├── TODO.md                       # Project vision, bug tracker, and development roadmap
└── 3d_visualizer.html            # Generated Three.js WebGL visualizer output
```

---

## 📄 License

This project is licensed under the **GNU Affero General Public License v3.0 (AGPL-3.0)**.

Copyright © 2026 Omid Teimory. All Rights Reserved. See [LICENSE](LICENSE) for full licensing terms.
