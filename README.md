```
  _________________________________________________________________________________________
 /                                                                                         \
|  [!] END-USER LICENSE AGREEMENT (EULA) & TERMS OF SERVICE [!]                             |
|                                                                                           |
|  WARNING: This software is a high-fidelity, advanced physics and penetration simulator.   |
|  Usage of this application is strictly restricted to recreational, educational, and       |
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

# MOP Simulator V3.0 - Autonomous AI Penetration Research Platform

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)
![Node.js](https://img.shields.io/badge/Node.js-v24-green.svg)
![MongoDB](https://img.shields.io/badge/MongoDB-Persisted-yellow.svg)
![AI Powered](https://img.shields.io/badge/AI-Autonomous_Research-purple.svg)
![License](https://img.shields.io/badge/License-AGPLv3-g.svg)
![Status](https://img.shields.io/badge/Physics-100%25%20Validated-orange.svg)

**MOP Simulator V3.0** has evolved from a standalone native binary into a **full-stack autonomous AI research platform**. It tightly couples a high-performance C++23 terminal ballistics simulation engine with a Node.js/Express automation backend and advanced LLM AI integration.

The platform is designed to autonomously hypothesize target geometries, execute massive multi-scenario penetrations (e.g., GBU-57 MOP, BLU-109, Orbital Kinetic Strikes), stream and aggregate telemetry to a MongoDB database, and automatically synthesize the findings into peer-reviewed-quality academic research articles.

---

## 🚀 Architecture & Ecosystem

```mermaid
flowchart TD
    subgraph AILayer["🧠 Autonomous AI Layer (src/AI)"]
        A1[Research Conductor] -->|Generates Scenarios| A2[aiClient.js]
        A3[Article Writer] -->|Synthesizes Findings| A2
    end

    subgraph NodeLayer["🌐 Node.js Orchestrator (src/Automation)"]
        N1[Express API / Controllers]
        N2[Simulation Runner]
        N3[(MongoDB - Result & Article Models)]
        A2 <-->|API Calls| N1
        N1 <--> N2
        N1 <--> N3
        N2 -->|Saves Telemetry| N3
    end

    subgraph CppLayer["⚙️ C++23 Physics Kernel (src/simulation)"]
        C1[main.cpp CLI]
        C2[Numerical Solvers<br/>RK4, Forrestal, WAPM, Hugoniot]
        C3[Telemetry Exporter<br/>JSON stdout]
        N2 -->|Spawns sim.exe (stdin)| C1
        C1 --> C2
        C2 --> C3
        C3 -->|Pipes back to Node| N2
    end
```

---

## 🧠 The Autonomous AI Research Pipeline

The V3.0 architecture introduces a two-phase autonomous pipeline:

### Phase 1: Autonomous Scenario Generation (`POST /research`)
1. **Hypothesis Generation**: The AI Research Conductor takes a research title (e.g., "Optimizing Casing Thickness for 70MPa Concrete").
2. **Matrix Sweeping**: The AI generates a highly varied parameter matrix for multiple simulation cycles (e.g., altering velocities, target layers, casing thickness).
3. **Headless Execution**: The Node.js `SimulationRunner` silently spawns the C++ kernel `mop_sim.exe` for each cycle.
4. **Telemetry Ingestion**: C++ computes sub-millisecond numerical integration and pipes massive JSON frames back to Node.js, which are immediately saved into MongoDB.

### Phase 2: Scientific Synthesis (`POST /article`)
1. **Data Aggregation**: The Node.js backend pulls all simulation results from MongoDB and computes advanced statistical metrics (mean/max penetration, standard deviations, failure rates, regime breakdowns).
2. **Article Generation**: The AI Article Writer ingests the statistical telemetry.
3. **Publishing**: It synthesizes a rigorous 4,000–12,000 word academic paper complete with an Abstract, Introduction, Methodology, Results & Discussion, and Conclusion.

---

## 🔬 Core Physics & Mathematical Framework

The native C++ simulation engine remains the heart of the project, integrating continuum mechanics, cavity expansion theory, and Hugoniot shock impedance matching.

### 1. Cavity Expansion & Deceleration Model (Two-Phase Forrestal)
For penetration into reinforced concrete and geological strata, the deceleration force $F_z$ is governed by cavity expansion dynamics:

$$ F_z = -\frac{\pi D^2}{4} \left( S f_c' + N \rho_t v^2 \right) $$

Where:
- $D$: Projectile diameter ($m$)
- $f_c'$: Dynamic Increase Factor (CEB-FIP DIF) adjusted compressive strength
- $S$: Empirical target strength multiplier ($S = 82.6 \cdot (f_c')^{-0.544}$)
- $N$: Nose shape coefficient ($\text{CRH}$)
- $\rho_t$: Target material density ($kg/m^3$)
- $v$: Instantaneous velocity ($m/s$)

### 2. Walker-Anderson Hydrodynamic Rod Erosion (WAPM)
At hypervelocity speeds ($v > 1200\ m/s$), when dynamic pressures exceed casing yield strength ($P_{dyn} > Y_p$). Interface velocity $u$ is given by Tate-Bernoulli:

$$ Y_p + \frac{1}{2} \rho_p (v - u)^2 = R_t + \frac{1}{2} \rho_t u^2 $$

### 3. Walker-Wasley Hugoniot Shock Initiation
Explosive shock initiation is evaluated by impedance matching shock Hugoniot jump conditions:

$$ U_s = C_0 + S U_p, \quad P = \rho_0 U_s U_p $$

Transmitted shock stress $P_{shock}$ and casing transit pulse duration $\tau$ evaluate critical initiation energy $P^2 \tau \ge E_c$.

---

## 🌐 Interactive 3D WebGL Physics Visualizer

The C++ engine natively exports `3d_visualizer.html`, providing:
- **100% Physics WebGL Rendering**: Driven frame-by-frame by telemetry.
- **US Standard Atmosphere 1976**: Inverse transform sampled sky dust particles based on barometric density.
- **Prandtl-Glauert Supersonic Shock Cones**: $\sin(\alpha) = 1/M$ attached in 3D matrix sync with the velocity vector.
- **Planck Blackbody Thermal Radiation**: Friction work elevates casing temperature. Radiation emission follows Planck's Law, shifting colors dynamically from dull red ($800\text{ K}$) to plasma ($2200\text{ K}+$).

---

## 🛠️ Installation & Setup

### Requirements
- **C++ Compiler**: GCC / MinGW-w64 with **C++23** support (`g++ >= 13.0`)
- **Node.js**: v20 or higher
- **Database**: MongoDB instance (local or Atlas)

### 1. Build the C++ Simulation Engine
```powershell
# Open terminal in project root
mingw32-make clean
mingw32-make
```

### 2. Setup Node.js & AI Environment
```powershell
cd src/Automation

# Install dependencies
npm install

# Create a .env file and add credentials
echo "MONGO_URI=mongodb://127.0.0.1:27017/mop-simulator" > .env
```

### 3. Start the Platform
```powershell
# Inside src/Automation
npm start
```
The server will run on `http://localhost:3000`. You can now hit the `/research` and `/article` REST endpoints to unleash the AI pipeline!

---

## 📜 License & Copyright

**Copyright (c) 2026 Omid Teimory. All Rights Reserved.**

Licensed under the GNU Affero General Public License v3.0 (AGPLv3). See `LICENSE` for details.
