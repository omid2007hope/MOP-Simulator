# 7. Glossary

// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

---

## 📖 Technical Terms, Symbols & Acronyms

### A
* **Alekseevskii-Tate Model**: A 1D hydrodynamic theory governing high-velocity impact and plastic erosion of long penetrators entering deformable targets based on modified Bernoulli pressure balance: $Y_p + \frac{1}{2}\rho_p(v-u)^2 = R_t + \frac{1}{2}\rho_t u^2$.
* **Angle of Attack (AoA, $\alpha$)**: The angle between a projectile's longitudinal geometric axis and its instantaneous velocity vector $\vec{v}$.
* **Area Moment of Inertia ($I_{area}$)**: A geometric property of the projectile cross-section ($m^4$) measuring its resistance to lateral bending and structural snapping.

---

### B
* **Bar Wave Speed ($c_{bar}$)**: The speed of an acoustic longitudinal elastic wave traveling through the penetrator casing: $c_{bar} = \sqrt{E / \rho_p}$.
* **Bending Moment ($M_b$)**: Lateral torque generated during oblique penetration due to asymmetric target pressure distributions.

---

### C
* **Caliber-Radius-Head (CRH)**: The non-dimensional ratio of ogive nose profile radius of curvature to projectile body diameter ($\text{CRH} = R_{nose} / D$).
* **Cavity Expansion Theory**: Continuum mechanics framework modeling the radial expansion of spherical or cylindrical cavities in elastic-plastic geomaterials under projectile entry.
* **CEB-FIP DIF (Dynamic Increase Factor)**: An empirical strain-rate multiplier formulation scaling the static compressive strength of concrete ($f_c'$) under dynamic impact rates.
* **Cratering Phase (Phase I)**: The initial entry depth interval ($z < 2D$) where unconfined target spallation and shear cone failure dominate.

---

### D
* **DBHT**: *Deeply Buried Hardened Target* — Subterranean defense facilities, reinforced bunkers, and tunnel complexes engineered to resist kinetic penetrators.
* **Dynamic Pressure ($P_{dyn}$)**: Stagnation pressure exerted on the projectile nose tip during flight and impact ($P_{dyn} = \frac{1}{2}\rho v^2$).

---

### E
* **Eglin Steel (ES-1)**: A high-strength, high-toughness steel alloy specifically formulated for bunker-defeating penetrator casings.
* **Explosive Critical Energy ($E_c$)**: The threshold value in the Walker-Wasley criterion ($P^2\tau \ge E_c$) beyond which impact shock waves induce premature explosive deflagration/detonation.

---

### F
* **Forrestal Cavity Expansion Model**: Semi-analytical formulation predicting axial deceleration forces for rigid ogive-nosed penetrators entering geological and concrete media.

---

### G
* **GBU-57 MOP**: *Massive Ordnance Penetrator* — A 13,608 kg precision-guided bunker buster munition designed to penetrate deep subterranean hardened targets.

---

### H
* **Hugoniot Equation of State (EOS)**: The locus of thermodynamic states achievable across a planar shock wave, represented by the linear jump relation $U_s = C_0 + S U_p$.
* **Hydrodynamic Penetration**: Penetration regime wherein impact pressures vastly exceed material flow strength ($P_{dyn} \gg Y_p$), causing both projectile and target to flow like fluid jets.

---

### J
* **J-Hook Failure**: Catastrophic structural snapping of a penetrator casing caused when lateral bending moments during oblique impact exceed the section yield strength.

---

### L
* **LibTorch**: The native C++ frontend for PyTorch, used in MOP Simulator V4.0 to execute $O(1)$ surrogate neural physics models.

---

### M
* **Mach Number ($M$)**: Non-dimensional ratio of instantaneous flight velocity to local atmospheric speed of sound ($M = v / c$).

---

### O
* **Obliquity Angle ($\theta$)**: The angle measured between the velocity vector and the inward normal vector of the target surface ($0^\circ = \text{normal/perpendicular strike}$).

---

### P
* **Particle Velocity ($U_p$)**: The velocity of target and projectile material particles immediately behind a propagating shock wave front.
* **Planck Blackbody Radiation**: Physics principle governing thermal emission colors as a function of penetrator casing temperature rise.
* **Prandtl-Glauert Shock Cone**: Conical supersonic wave boundary with half-angle $\alpha = \arcsin(1/M)$ attached to munitions moving at $M > 1.0$.

---

### R
* **Rigid Penetration**: Penetration regime where projectile dynamic yield strength exceeds target dynamic resistance, resulting in zero projectile mass erosion.
* **Runge-Kutta 4th Order (RK4)**: A 4th-order iterative numerical integrator used to solve differential equations of motion for atmospheric drop and underground trajectories.

---

### S
* **Shock Wave Velocity ($U_s$)**: The propagation velocity of high-pressure shock wave fronts traveling through the penetrator casing and target media.

---

### T
* **Tunneling Phase (Phase II)**: Subterranean penetration regime ($z \ge 2D$) where radial cavity expansion governs constant-diameter tunnel formation.

---

### U
* **US Standard Atmosphere (1976)**: Standardized atmospheric model defining temperature, barometric pressure, air density, and speed of sound up to $86\text{ km}$ altitude.

---

### W
* **Walker-Anderson Penetration Model (WAPM)**: Time-dependent analytical model accounting for dynamic interface erosion and deceleration of long rod penetrators.
* **Walker-Wasley Criterion**: Shock initiation threshold relating transmitted shock pressure squared times pulse duration to explosive sensitivity: $E_{shock} = P_{trans}^2 \cdot \tau$.

---

### Y
* **Yield Strength ($Y_p$, $\sigma_{yield}$)**: Material stress threshold beyond which permanent plastic deformation or hydrodynamic flow occurs.

---

## 🧭 Navigation

* [Back to Documentation Index](README.md)
* [Explore 2. System Architecture](02-system-architecture.md)
* [Explore 2.1.1 Physics Models & Numerical Methods](02-01-01-physics-models-and-numerical-methods.md)
