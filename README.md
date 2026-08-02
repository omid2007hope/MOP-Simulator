```
  _________________________________________________________________________________________
 /                                                                                         \
|  [!] END-USER LICENSE AGREEMENT (EULA) & TERMS OF SERVICE [!]                             |
|                                                                                           |
|  WARNING: This software is a high-fidelity, advanced physics and penetration simulator.   |
|  Usage of this application is strictly restricted to recreational, educational, and      |
|  hobbyist purposes. Due to the extreme accuracy and sensitive nature of the simulated     |
|  models, any unauthorized, commercial, or malicious application may result in severe      |
|  legal consequences.                                                                      |
|                                                                                           |
|  DISCLAIMER OF WARRANTY: This software is provided "AS IS", without warranty of any       |
|  kind, express or implied.                                                                |
|  LIMITATION OF LIABILITY: In no event shall the author(s) be liable for any claim,        |
|  damages, or other liability arising from, out of, or in connection with the software     |
|  or the use or other dealings in the software.                                            |
|  By using this repository, you acknowledge that this tool is not certified for real-world |
|  engineering, defense analysis, or physical destructive testing.                          |
 \_________________________________________________________________________________________/
```

# C++ Impact Physics & Terminal Ballistics Penetration Simulator v2.8

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)
![License](https://img.shields.io/badge/License-AGPLv3-g.svg)
![Build](https://img.shields.io/badge/Build-MinGW%20%2F%20GCC-green.svg)
![Status](https://img.shields.io/badge/Physics-Validated-orange.svg)

A high-performance C++23 terminal ballistics simulation engine designed to model high-velocity kinetic impact, deep target penetration, structural failure, and thermodynamic behavior of heavy penetrators (e.g., GBU-57 MOP, BLU-109, and Orbital Tungsten Rods) into multi-layered geological and reinforced concrete structures.

Includes an automated, zero-dependency **Interactive WebGL 3D Visualization Pipeline** generated directly from C++ trajectory integrations.

---

## 🔬 Core Physics & Mathematical Framework

The simulation engine integrates continuum mechanics, cavity expansion theory, and dynamic structural failure models step-by-step through depth.

### 1. Cavity Expansion & Deceleration Model (Modified Forrestal / Poncelet)
For penetration into reinforced concrete and geological strata, the deceleration force $F_z$ is governed by cavity expansion dynamics:

$$ F_z = -\frac{\pi D^2}{4} \left( S f_c' + N \rho_t v^2 \right) $$

Where:
- $D$: Projectile diameter ($m$)
- $f_c'$: Unconfined compressive strength of the current target layer ($Pa$)
- $S$: Empirical target strength multiplier ($S = 82.6 \cdot (f_c')^{-0.544}$)
- $N$: Nose shape coefficient derived from Caliber Radius Head ($\text{CRH}$)
- $\rho_t$: Target material density ($kg/m^3$)
- $v$: Instantaneous velocity ($m/s$)

### 2. Hydrodynamic Penetration Limit (Alekseevskii-Tate Model)
At hypervelocity speeds ($v > 1200\ m/s$), dynamic pressures exceed the yield strength of both the target and the penetrator casing, shifting the regime into hydrodynamic fluid-like erosion:

$$ P_{dyn} = \frac{1}{2} \rho_t v^2 $$

$$ L_{max} = L_0 \sqrt{\frac{\rho_p}{\rho_t}} $$

Where $\rho_p$ is casing density and $L_0$ is original penetrator length.

### 3. Failure Regimes & Damage Mechanics
The simulator continuously evaluates three primary destruction regimes during penetration:
- **Pressure Yield (Crush)**: Casing walls collapse when dynamic pressure exceeds material yield strength ($P_{dyn} > \sigma_y$).
- **Structural Bending Moment (J-Hooking / Snap)**: Oblique impacts ($\theta > 0$) with angle of attack ($\alpha$) generate asymmetric normal forces, creating a bending moment $M_{bending}$. If stress exceeds $\sigma_y \cdot Z$ (where $Z$ is the section modulus $I/y$), the casing snaps.
- **Thermodynamic Filler Detonation (Cook-off)**: Sliding friction thermal energy $Q_{fric} = \mu \cdot F_n \cdot v \cdot \Delta t$ transfers into the explosive core. If filler temperature exceeds critical ignition thresholds, early detonation occurs mid-penetration.

---

## 📁 Repository & Architecture Layout

```text
MOP Simulator/
├── assets/
│   └── visualizer_template.html   # Three.js 3D WebGL visualization template
├── bin/
│   ├── sim.exe                    # Production CLI simulator binary (V2.8)
│   └── test_simulation.exe        # Automated physics unit testing binary
├── build/                         # Object files compiled during build
├── data/
│   ├── projectiles.json           # Presets for GBU-57 MOP, BLU-109, Tungsten Rods
│   └── targets.json               # Layered targets (Concrete, Soil, Granite)
├── documents/
│   ├── Ai/                        # AI coding directives, architecture & workflow
│   ├── commands/                  # Command references (compiling, testing, deploy)
│   ├── contribution/              # Contributor guidelines, Roadmap & JSON schema
│   ├── learning/                  # Post-mortems, console lifecycle & WebGL pipeline
│   └── physic/                    # Physics equations, Poncelet math & yield limits
├── include/
│   ├── config_loader.hpp          # JSON database parser interfaces
│   ├── default.hpp                # Hardcoded default presets (GBU-57, Granite)
│   ├── nlohmann/json.hpp          # Single-header JSON library
│   └── simulation.hpp             # Physics structures & simulator engine interface
├── src/
│   ├── config_loader.cpp          # Target/Projectile JSON loading implementation
│   ├── main.cpp                   # Application entry, EULA logic & CLI menus
│   └── simulation.cpp             # Core numerical integrator & 3D HTML exporter
├── tests/
│   └── test_simulation.cpp        # C++ unit test suite covering 4 core physics regimes
├── CMakeLists.txt                 # CMake project configuration (V2.8.0)
├── Makefile                       # MinGW / GCC C++23 build pipeline
└── README.md                      # Primary documentation
```

---

## 🛠️ Build & Compilation

### Requirements
- **Compiler**: GCC / MinGW-w64 with **C++23** support (`g++ >= 13.0`)
- **Build System**: `mingw32-make` or `make` or `CMake`

### 1. Compile Main Binary & Tests (`Makefile`)
Open PowerShell / Terminal in the project root:

```powershell
# Clean previous build artifacts
mingw32-make clean

# Build production executable (bin/sim.exe)
mingw32-make

# Build and run physics verification test suite (bin/test_simulation.exe)
mingw32-make test
```

### 2. Alternative Build via CMake
```powershell
mkdir build
cd build
cmake ..
cmake --build .
```

---

## 🚀 Usage Modes

Upon executing `bin/sim.exe`, users are presented with the mandatory EULA consent check followed by 3 execution modes:

```text
Select Simulation Mode:
  [1] Run Standard GBU-57 MOP Presets (Mach 1.0 to Mach 10.4)
  [2] Interactive Custom Input (cin values for mass, velocity, density, etc.)
  [3] Orbital Kinetic Strike Preset ("Rods from God" Tungsten Penetrators)
```

### Interactive Output
The simulator generates:
1. **Console Terminal Report**: Instantaneous energy breakdown (GJ), maximum dynamic pressure (GPa), entry/exit velocities, penetration depth ($m$), and failure regime status.
2. **ASCII Cross-Section Plot**: Character-based depth slice of target cratering and projectile stopping location.
3. **HTML WebGL 3D Interactive Visualizer**: Generates `output_3d_visualizer.html` in the project root for 3D graphics rendering in any web browser.

---

## 🌐 Interactive 3D WebGL Visualizer

The engine automatically exports `output_3d_visualizer.html` combining:
- **Three.js Graphics Rendering**: Interactive orbit controls, multi-layered target translucency, depth markers, and shockwave propagation rings.
- **Dynamic Physics Charts**: Real-time Chart.js telemetry plotting Velocity vs. Depth and Dynamic Pressure vs. Depth.

---

## 🧪 Unit Test Suite Verification

Run `mingw32-make test` to execute 4 automated physics validation tests:
1. **Subsonic Time-Integrated Penetration** ($340\ m/s$ into reinforced concrete — intact survival).
2. **Hypervelocity Hydrodynamic Failure** ($1500\ m/s$ impact — pressure yield crush & Tate depth calculation).
3. **Orbital Tungsten Kinetic Rod Strike** ($3400\ m/s$, $0$ explosive filler — hypervelocity kinetic erosion).
4. **Oblique Impact Bending Structural Failure** ($30^\circ$ obliquity, $5^\circ$ AoA — J-Hook bending snap detection).

---

## 📜 License & Copyright

**Copyright (c) 2026 Omid Teimory. All Rights Reserved.**

Licensed under the GNU Affero General Public License v3.0 (AGPLv3). See `LICENSE` for details.
