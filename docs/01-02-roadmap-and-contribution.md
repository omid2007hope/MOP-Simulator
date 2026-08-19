# 1.2 Project Roadmap & Contribution Guide

// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

---

## 🗺️ Project Roadmap

### ✅ Completed Milestones (V3.0 & V3.5.0)

1. **Full-Stack Autonomous AI Research Platform**:
   - Automated scenario formulation using Google Gemini 2.5 Flash (`researchConductor`).
   - Headless `--json-input` execution flag in C++23 native core.
   - High-throughput asynchronous streaming parser with 1,000-frame batch MongoDB insertion.
   - Closed-loop multi-cycle research runs (`POST /research`).
   - Automated publication synthesizer (`POST /article`) outputting 4,000–12,000 word academic papers.

2. **Terminal Ballistics Continuum Mechanics Core**:
   - Walker-Anderson hydrodynamic rod erosion model (WAPM / Alekseevskii-Tate).
   - Walker-Wasley shock initiation with Hugoniot impedance matching ($U_s = C_0 + S U_p$).
   - CEB-FIP dynamic increase factor (DIF) for strain-rate concrete scaling.
   - US Standard Atmosphere (1976) barometric model with supersonic shock transition detection.
   - Two-phase Forrestal cavity expansion model (surface cratering + deep tunneling).

3. **Multi-Platform Visualization**:
   - Interactive Three.js WebGL visualizer (`3d_visualizer.html`) with Planck blackbody radiation and Prandtl-Glauert shock cones.
   - Unreal Engine 5 integration (`UnreaEngine`) with async simulation bridge and blueprint nodes.

---

### 🔮 Future Milestones (V4.0+)

1. **Embedded C++ Machine Learning Core (LibTorch)**:
   - **Surrogate Neural Physics**: Replacing iterative RK4 integration loops with $O(1)$ deep neural networks (DNN) compiled natively in C++ via PyTorch C++ (LibTorch).
   - **Reinforcement Learning Smart Fuze (RL)**: Microsecond-precision detonation triggering based on real-time $g$-force, cavity detection, and shock pressure feedback.
   - *Reference: `src/MachineLearning/MachineLearning.md`*.

2. **2D/3D Trajectory & Path Deviation**:
   - Pitch and yaw bending moments as a function of penetration depth.
   - Trajectory curvature and deflection when attacking multi-layer geological strata at high obliquity angles ($> 30^\circ$).

3. **Stratified Volumetric Voxel Excavation**:
   - Volumetric 3D voxel terrain degradation to visualize complex cratering, rebar ripping, and cavity collapse in real time.

---

## 🤝 Contribution Guidelines

We welcome contributions from terminal ballistics researchers, computational physicists, C++ engineers, and full-stack developers.

### Contribution Workflow

1. **Fork the Repository**:
   Create a feature branch from `main`:
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Code Standards & Architecture**:
   - Adhere strictly to the simplicity-first philosophy: *"An idiot admires complexity; a genius admires simplicity."*
   - Keep C++ code modern (C++23), clean, and warning-free under `-Wall -Wextra -O2`.
   - Format all C++ code using `.clang-format` before submitting.
   - Format JavaScript code using Prettier (`.prettierrc`).
   - See [5.2 Code Style & Formatting Standards](05-02-code-style-and-formatting-standards.md) for detailed rules.

3. **Testing Requirements**:
   - Any modifications to the physics kernel must pass the complete unit test suite:
     ```powershell
     cd backend
     mingw32-make test
     ```
   - If adding new physical formulations or materials, include corresponding test assertions in `backend/tests/test_simulation.cpp`.

4. **Submitting Pull Requests**:
   - Provide a clear, detailed PR description outlining the mathematical rationale, changes made, and benchmark results.
   - Link any relevant research papers or literature backing physical changes.

---

## 🧭 Navigation

* [Back to 1. Project Overview](01-overview.md)
* [Proceed to 2. System Architecture](02-system-architecture.md)
* [Explore 5.2 Code Style & Formatting Standards](05-02-code-style-and-formatting-standards.md)
