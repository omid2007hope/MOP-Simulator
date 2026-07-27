# MOP - Massive Ordnance Penetrator Bomb Simulator

A C++ physics and penetration mechanics simulator for high-mass earth-penetrating ordnance like the **GBU-57 Massive Ordnance Penetrator (MOP)**. This project models terminal ballistic impact physics, structural casing integrity, and hydrodynamic soil/concrete penetration limits, featuring both a terminal ASCII cross-section renderer and an automatically generated **Three.js WebGL 3D interactive visualizer**.

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

## 🎓 C++ Learning Concepts Highlighted

This repository serves as a practical codebase for reviewing C++ fundamentals and design patterns:

1. **Object-Oriented Programming (OOP)**: Clean encapsulation of physical properties using `struct` definitions (`Target`, `Projectile`, `ImpactScenario`, `SimulationResult`) and an `ImpactSimulator` class.
2. **Standard Template Library (STL)**: Extensive use of `std::vector`, `std::string`, `std::stringstream`, `<iomanip>` for tabular formatting, and `<cmath>`.
3. **File I/O Stream Manipulation**: C++ `std::ofstream` to dynamically generate standalone HTML/JavaScript WebGL files using raw string literals (`R"(...)"`).
4. **Input Sanitization**: Terminal buffer clearing (`clearCinBuffer()`) using `std::cin.clear()` and `std::cin.ignore()` for safe CLI interaction.

---

## 🛠️ How to Build and Run

### Prerequisites
- Any modern C++ compiler supporting C++11 or later (`g++`, `clang++`, or MSVC `cl`).

### Compilation

Using **GCC / MinGW**:
```bash
g++ -O2 Simulation/sim.cpp -o Simulation/sim.exe
```

Using **Clang**:
```bash
clang++ -O2 Simulation/sim.cpp -o Simulation/sim.exe
```

Using **MSVC Developer Command Prompt**:
```cmd
cl /EHsc /O2 Simulation/sim.cpp /Fe:Simulation/sim.exe
```

### Running the Simulator

```bash
./Simulation/sim.exe
```

1. **Select Mode**:
   - `[1]` Run Standard GBU-57 MOP Presets (Mach 1.0 to Mach 10.4).
   - `[2]` Interactive Custom Parameter Input.
2. View the console summary report and ASCII cross-sections.
3. Open `Simulation/3d_visualizer.html` in any web browser to explore the interactive 3D WebGL scene!

---

## 📁 Repository Structure

```
MOP Simulator/
├── .clang-format           # Code formatting configuration
├── License.md              # Software license & terms
├── README.md               # Project documentation
└── Simulation/
    ├── sim.cpp             # Main C++ source code (Physics engine & HTML generator)
    ├── sim.exe             # Compiled binary
    └── 3d_visualizer.html  # Generated Three.js WebGL visualizer
```

---

## 📄 License

Copyright © 2026 Omid Teimory. All Rights Reserved. See [License.md](License.md) for licensing terms.
