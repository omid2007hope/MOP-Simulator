# MOP - Massive Ordnance Penetrator Bomb Simulator

A C++ physics and penetration mechanics simulator for high-mass earth-penetrating ordnance like the **GBU-57 Massive Ordnance Penetrator (MOP)**. This project models terminal ballistic impact physics, structural casing integrity, and hydrodynamic soil/concrete penetration limits, featuring both a terminal ASCII cross-section renderer and an automatically generated **Three.js WebGL 3D interactive visualizer**.

---

## 🛠️ How to Build and Run

### Prerequisites
- Any modern C++ compiler supporting C++17 or later (`g++`, `clang++`, or MSVC `cl`).
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
g++ -std=c++17 -O2 -Iinclude src/main.cpp src/simulation.cpp -o bin/sim.exe
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
   - `[1]` Run Standard GBU-57 MOP Presets (Mach 1.0 to Mach 10.4).
   - `[2]` Interactive Custom Parameter Input.
   - `[3]` Orbital Kinetic Strike Preset ("Rods from God").
2. View the console summary report and ASCII cross-sections.
3. The simulator loads `assets/visualizer_template.html` and injects dynamic simulation results to output `3d_visualizer.html` in the root directory.

#### 🌐 Viewing `3d_visualizer.html`
Due to browser CORS security restrictions when loading CDN scripts on local `file://` URLs, use a local web server to view the generated 3D scene:

```bash
# Using Node.js (Recommended)
npx serve .

# Or using Python
python -m http.server 8080
```
Open `http://localhost:3000/3d_visualizer.html` (or `http://localhost:8080/3d_visualizer.html`) in your browser.

### Running Unit Tests

To run the automated physics and penetration regime unit tests:
```bash
make test
# or on Windows with MinGW:
mingw32-make test
```

---

## 🌟 Key Features

- **Physics-Based Terminal Ballistics**: Evaluates kinetic energy ($E_k$), dynamic impact pressure ($P_{dyn}$), and fluid penetration limits.
- **Alekseevskii-Tate Hydrodynamic Model**: Calculates maximum hydrodynamic penetration depth ($P = L \cdot \sqrt{\rho_p / \rho_t}$) in target media (e.g., hardened concrete).
- **Casing Failure & Regimes**: Distinguishes between **Rigid Body Penetration** (deep underground drilling) and **Hydrodynamic / Hypervelocity Casing Failure** (surface crushing/detonation).
- **Dual Visualizer**:
  - **Terminal ASCII 3D Cross-Sections**: Instant visual output in the command line for each scenario.
  - **Interactive 3D WebGL HTML Generation**: Generates `3d_visualizer.html` containing Three.js visualizer with camera controls, orbit navigation, and HUD metrics.
- **Interactive CLI & Preset Scenarios**: Choose standard high-altitude GBU-57 drop presets or input custom projectile dimensions, material densities, casing yield strength, and impact velocities.

---

## 🌐 Web Application (Standalone)

The simulator is also available as a **self-contained web application** — no compilation or build step needed.

### Quick Start

1. Open `index.html` in any modern web browser (Chrome, Firefox, Edge, Safari).
2. Select a simulation mode:
   - **GBU-57 MOP Standard** — 4 preset altitude drop scenarios
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
- Payload penetrates deep underground ($40 - 60+ \text{ meters}$) before detonation.

### 4. Alekseevskii-Tate Hydrodynamic Penetration Limit
$$P = L \cdot \sqrt{\frac{\rho_p}{\rho_t}}$$
Where:
- $L$ = Projectile length ($\text{meters}$)
- $\rho_p$ = Casing density ($\text{kg/m}^3$)
- $\rho_t$ = Target density ($\text{kg/m}^3$)

---

## 📁 Repository Structure

```text
MOP Simulator/
├── index.html               # Standalone web application (UI + Sim + 3D Visualizer)
├── assets/
│   └── visualizer_template.html # External HTML/JS template for 3D visualizer generation
├── include/
│   └── simulation.hpp       # Public API headers (Data structs & class declarations)
├── src/
│   ├── simulation.cpp       # Core physics engine & 3D HTML generator implementation
│   └── main.cpp             # CLI application entry point & user menu
├── tests/
│   └── test_simulation.cpp  # Automated unit test suite for ballistics & failure regimes
├── bin/                     # Output directory for compiled executables
├── build/                   # Intermediate compilation object files
├── Makefile                 # Standard build script for GCC/Clang/MinGW
├── CMakeLists.txt           # Standard CMake build configuration
├── .clang-format            # Code formatting configuration
├── License.md               # Software license & terms
├── README.md                # Project documentation
└── 3d_visualizer.html       # Generated Three.js WebGL visualizer output
```

---

## 📄 License

Copyright © 2026 Omid Teimory. All Rights Reserved. See [License.md](License.md) for licensing terms.
