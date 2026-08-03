# 100% Physics WebGL 3D Visualization Pipeline: C++ to Browser Bridge

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

---

## 1. Zero-Dependency Web Architecture

Most desktop software that renders 3D graphics either embeds a heavy web browser instance (like Electron/CEF) or runs a local web server (like Node.js/Express) to serve static files.

The **MOP Simulator** uses a much lighter and faster architectural pattern:
1. The C++ core physics engine performs high-precision Runge-Kutta numerical integration on the CPU.
2. The simulation outputs full state trajectories (`drop_frames` and `penetration_frames`).
3. C++ acts as a **template engine**, injecting structured JSON telemetry directly into an offline HTML WebGL template file (`assets/visualizer_template.html`).
4. The output `3d_visualizer.html` can be opened in **any standard web browser** directly from disk with zero setup!

---

## 2. Telemetry Serialization in C++

In [`src/simulation.cpp`](file:///h:/Code/MyOwn/Main/Best/MOP%20Simulator/src/simulation.cpp#L706-L768), C++ constructs JSON arrays by serializing vector frames:

```cpp
std::stringstream dropFramesJson;
dropFramesJson << "[";
for (size_t j = 0; j < r.drop_frames.size(); ++j) {
    const auto& f = r.drop_frames[j];
    dropFramesJson << "{t:" << f.time << ",y:" << f.depth 
                   << ",v:" << f.velocity << ",m:" << f.mach
                   << ",h:" << f.heat << ",g:" << f.g_force
                   << ",rl:" << f.remaining_length
                   << ",obliquity_deg:" << f.obliquity_deg
                   << ",sb:" << (f.is_sonic_boom ? "true" : "false") << "}";
    if (j + 1 < r.drop_frames.size()) dropFramesJson << ",";
}
dropFramesJson << "]";
```

---

## 3. 100% Physics-Based WebGL Render Subsystems

### A. Planck Blackbody Thermal Radiation (`computePlanckBlackbodyColor`)
Friction and hydrodynamic ablation heating $T$ (in Kelvin) drive Planck blackbody radiation and Wien's displacement law ($\lambda_{\max} T = 2.89777 \times 10^{-3}\text{ m}\cdot\text{K}$):
- $< 500\text{ K}$: Base casing metal
- $500\text{ K} - 900\text{ K}$: Incipient red-heat
- $900\text{ K} - 1500\text{ K}$: Cherry red to bright orange
- $1500\text{ K} - 2200\text{ K}$: Incandescent yellow-white
- $> 2200\text{ K}$: Radiant plasma white-blue
- Emission intensity scales with $T^4$ (Stefan-Boltzmann Law).

### B. Supersonic Prandtl-Glauert Mach Shock Cones
Mach cone half-angle obeys $\sin(\alpha) = 1/M$ for $M > 1.0$. Vapor opacity and density scale with dynamic pressure $q = \frac{1}{2} \rho v^2$ and local air density $\rho(y)$.

### C. US Standard Atmosphere 1976
Air density follows $\rho(y) = \rho_0 e^{-k \cdot y}$. Sky dust particles use inverse transform sampling to match Earth's barometric density profile.

### D. School Ruler Measuring Sub-Divisions
Depth and altitude rulers include precision sub-divisions (1m minor ticks, 5m medium ticks, and 10m major ticks + text labels) for both air and underground target layers.

### E. Web Audio USA National Anthem Synthesizer
Synthesizes "The Star-Spangled Banner" in real time using browser `AudioContext` polyphonic oscillators as the bomb drop animation begins.

---

## 4. Benefits for C++ Developers

- **No Third-Party Web Server Dependencies:** Users don't need `npm`, `node`, `python`, or `http-server`.
- **Portable Output:** The resulting `.html` file is a single self-contained document that can be emailed or archived.
- **High Performance:** C++ does heavy math calculation on CPU in microseconds; JavaScript handles smooth 60 FPS GPU rendering in Three.js.
