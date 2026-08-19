# 2.1.1 Physics Models & Numerical Methods

// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

---

## 🔬 Mathematical & Physical Formulations

The **MOP Simulator** continuum mechanics engine models the complete lifecycle of high-velocity penetrating munitions through two sequential physical phases:
1. **Atmospheric Free-Fall Drop Phase** (Kinematics & Compressible Aerodynamics)
2. **Subterranean Ground Penetration Phase** (Continuum Cavity Expansion, Hydrodynamics & Shock Thermodynamics)

```mermaid
flowchart LR
    subgraph DropPhase["Phase 1: Atmospheric Free-Fall"]
        A[High-Altitude Release] --> B[US Standard Atmosphere 1976]
        B --> C[CRH Compressibility Drag Cd]
        C --> D[4th-Order Runge-Kutta RK4]
        D --> E[Sonic Boom Detection]
    end

    subgraph ImpactTransition["Surface Impact Transition"]
        E --> F[Hugoniot Shock Impedance Matching]
        F --> G[Walker-Wasley Explosive Initiation]
    end

    subgraph PenetrationPhase["Phase 2: Subterranean Penetration"]
        G --> H[Two-Phase Forrestal Model<br/>Crater z<2D | Tunnel z>=2D]
        H --> I[CEB-FIP Dynamic Increase Factor DIF]
        I --> J[Walker-Anderson Hydrodynamic Erosion WAPM]
        J --> K[Frictional Heating & Thermal Ablation]
        K --> L[Oblique Bending Stress & J-Hook Assessment]
    end
```

---

## 1. Atmospheric Free-Fall & Aerodynamics

The projectile is released from an aircraft at altitude $y_0$ with velocity $v_0$ and flight path angle $\gamma$. Trajectory states are computed using a **4th-Order Runge-Kutta (RK4)** numerical integrator with time-step $\Delta t = 0.01\text{ s}$.

### Equations of Motion:

$$\frac{dv_x}{dt} = -\frac{F_{d,x}}{m} + a_{guidance,x}$$

$$\frac{dv_y}{dt} = -g - \frac{F_{d,y}}{m}$$

$$\frac{dy}{dt} = v_y$$

$$\frac{dx}{dt} = v_x$$

Where $F_d = \frac{1}{2} \rho_{air}(y) \cdot v^2 \cdot A \cdot C_d(M)$ and $A = \frac{\pi D^2}{4}$.

---

### US Standard Atmosphere (1976) Model

Ambient thermodynamic properties are dynamically evaluated as a function of geometric altitude $y$:

$$\text{Temperature: } T(y) = T_0 - L \cdot y \quad (\text{for } y \le 11,000\text{ m}, L = 0.0065\text{ K/m}, T_0 = 288.15\text{ K})$$

$$\text{Air Density: } \rho_{air}(y) = \rho_0 \cdot \exp\left(-\frac{M_{air} \cdot g \cdot y}{R \cdot T(y)}\right)$$

$$\text{Speed of Sound: } c(y) = \sqrt{\gamma_{air} \cdot \frac{R}{M_{air}} \cdot T(y)}$$

Where:
* $\rho_0 = 1.225\text{ kg/m}^3$ (Sea-level density)
* $M_{air} = 0.0289644\text{ kg/mol}$ (Molar mass of air)
* $R = 8.31432\text{ J/(mol}\cdot\text{K)}$ (Universal gas constant)
* $\gamma_{air} = 1.4$ (Adiabatic index)

---

### Caliber-Radius-Head (CRH) Aerodynamic Drag

The drag coefficient $C_d(M)$ is determined by the nose Caliber-Radius-Head ratio ($\text{CRH} = \frac{R_{nose}}{D}$):

$$C_{d,\text{base}} = \frac{8 \cdot \text{CRH} - 1}{24 \cdot \text{CRH}^2}$$

Compressibility corrections across Mach regimes:

$$C_d(M) = \begin{cases} 
C_{d,\text{base}} & M < 0.8 \quad (\text{Subsonic}) \\
C_{d,\text{base}} \cdot \left(1 + 2.5(M - 0.8)\right) & 0.8 \le M \le 1.2 \quad (\text{Transonic}) \\
\frac{C_{d,\text{base}}}{\sqrt{M^2 - 1}} + \Delta C_{d,\text{wave}} & M > 1.2 \quad (\text{Supersonic})
\end{cases}$$

When $v \ge c(y)$, a supersonic **sonic boom event** is flagged, recording the exact timestamp $t_{boom}$ and altitude $y_{boom}$.

---

## 2. Shock Impedance Matching & Walker-Wasley Initiation

Upon striking the target surface, high-pressure shock waves propagate through the casing and target.

### Linear Shock Hugoniot Jump Conditions:

$$U_s = C_0 + S \cdot U_p$$

At the impact interface, normal stress and velocity continuity dictate:

$$\text{Particle Velocity: } U_p = \frac{v_{imp}}{1 + \sqrt{\frac{\rho_t}{\rho_p}}}$$

$$\text{Peak Shock Pressure: } P_{shock} = \rho_p \cdot (C_{0,p} + S_p U_p) \cdot U_p$$

---

### Walker-Wasley Explosive Critical Initiation

The shock wave transmits through the casing wall of thickness $t_w$ with transmitted stress $P_{trans} \approx 0.25 \cdot P_{shock}$ and pulse duration $\tau = \frac{2 t_w}{U_{s,p}}$.

Critical initiation energy flux:

$$E_{shock} = P_{trans}^2 \cdot \tau$$

$$\text{Explosive Status} = \begin{cases} 
\text{Survives (Intact)} & E_{shock} < E_c \\
\text{Premature Detonation / Deflagration} & E_{shock} \ge E_c
\end{cases}$$

Where $E_c$ is the critical initiation energy threshold ($E_c \approx 3.0 \times 10^{15}\text{ Pa}^2\cdot\text{s}$ for insensitive high explosives).

---

## 3. Subterranean Deceleration: Two-Phase Forrestal Model

Penetration resistance into geological and concrete strata follows Forrestal's semi-analytical cavity expansion equations.

```
                         [Impact Velocity v_imp]
                                    │
                                    ▼
        ══════════════════════════════════════════════════ [Target Surface]
         \       Cratering Phase (Depth z < 2D)         /
          \   F_z = 0.5 * pi * D^2 * sigma_s * (z / 2D) /
           \───────────────────────────────────────────/
            │                                         │
            │   Tunneling Phase (Depth z >= 2D)       │
            │   F_z = - (pi*D^2 / 4) * (S*f_c' + N*rho*v^2)
            │                                         │
            ▼                                         ▼
```

### Phase I: Surface Cratering ($z < 2D$)

In the entry zone, unconfined spallation dominates:

$$F_z(z) = \frac{\pi D^2}{2} \cdot \sigma_s \cdot \left(\frac{z}{2D}\right)$$

### Phase II: Deep Tunneling ($z \ge 2D$)

Beyond two projectile diameters, spherical cavity expansion governs deceleration:

$$F_z(v) = -\frac{\pi D^2}{4} \left( S \cdot f_c'(\dot{\varepsilon}) + N \cdot \rho_t \cdot v^2 \right)$$

Where:
* $S = 82.6 \cdot (f_c')^{-0.544}$ (Empirical target resistance factor)
* $N = \frac{8 \cdot \text{CRH} - 1}{24 \cdot \text{CRH}^2}$ (Nose geometry coefficient)
* $\rho_t$ is target density ($kg/m^3$)

---

### CEB-FIP Dynamic Increase Factor (DIF)

Dynamic strain rates under hyper-velocity impact strengthen the concrete matrix:

$$f_c'(\dot{\varepsilon}) = f_{c,static}' \cdot \text{DIF}$$

$$\text{DIF} = \begin{cases} 
\left(\frac{\dot{\varepsilon}}{\dot{\varepsilon}_0}\right)^{1.026 \alpha_s} & \dot{\varepsilon} \le 30\text{ s}^{-1} \\
\gamma_s \left(\frac{\dot{\varepsilon}}{\dot{\varepsilon}_0}\right)^{1/3} & \dot{\varepsilon} > 30\text{ s}^{-1} 
\end{cases}$$

Where $\dot{\varepsilon}_0 = 30 \times 10^{-6}\text{ s}^{-1}$, $\alpha_s = (5 + 9 f_{c,static}'/10)^{-1}$, and $\gamma_s = 10^{6.156 \alpha_s - 2}$.

---

## 4. Hydrodynamic Rod Erosion (Walker-Anderson Model / WAPM)

When dynamic stagnation pressure exceeds casing dynamic yield strength ($P_{dyn} = \frac{1}{2} \rho_t v^2 > Y_p$), rigid penetration ceases and the munition undergoes hydrodynamic plastic erosion.

### Modified Alekseevskii-Tate Momentum Balance:

$$Y_p + \frac{1}{2} \rho_p (v - u)^2 = R_t + \frac{1}{2} \rho_t u^2$$

Where $u$ is interface penetration velocity and $R_t \approx 3.5 \cdot f_c'$ is target dynamic resistance.

$$\text{Rod Length Erosion Rate: } \frac{dL}{dt} = -(v - u)$$

$$\text{Mass Loss Rate: } \frac{dm}{dt} = \rho_p \cdot A \cdot \frac{dL}{dt}$$

$$\text{Theoretical Alekseevskii-Tate Hydrodynamic Limit: } P_{hydro} = L_0 \sqrt{\frac{\rho_p}{\rho_t}}$$

---

## 5. Frictional Thermodynamics, Heating & Ablation

Frictional shear work along the borehole boundary generates intense thermal energy flux:

$$\dot{Q}_{fric} = \mu_{fric} \cdot F_{tunnel} \cdot v$$

$$\text{Temperature Rise Rate: } \frac{dT}{dt} = \frac{\dot{Q}_{fric}}{m(t) \cdot c_p}$$

When casing temperature reaches the melting point $T \ge T_{melt} \approx 1800\text{ K}$, latent heat of fusion $L_f = 272,000\text{ J/kg}$ governs mass ablation, resulting in thermal casing destruction.

---

## 6. Oblique Impact Mechanics & Structural J-Hook Snapping

During oblique impacts (obliquity angle $\theta > 0^\circ$ or angle of attack $\alpha \ne 0^\circ$), asymmetric normal pressure exerts a lateral bending moment:

$$M_b = F_{asym} \cdot L_{moment}$$

$$\text{Maximum Bending Stress: } \sigma_b = \frac{M_b \cdot (D/2)}{I_{area}}$$

Where $I_{area}$ is the area moment of inertia. If $\sigma_b > \sigma_{yield}$, catastrophic casing structural snapping (**J-Hook Failure**) occurs, terminating penetration.

---

## 7. Sequential Salvo Synergy Mechanics

For multi-bomb salvos (e.g. Operation Midnight Hammer), subsequent munitions attack the pre-existing shaft:

$$z_{start, n} = \sum_{i=1}^{n-1} z_{breach, i}$$

Synergistic advantages:
1. **Pre-Cratered Entry**: Eliminates Phase I cratering resistance ($F_z = 0$ until $z \ge z_{start}$).
2. **Strata Degradation**: Pulverized residual compressive strength is reduced by $40\text{--}60\%$.
3. **Conserved Kinetic Energy**: Penetrators enter deep strata at near-terminal velocities.

---

## 🧭 Navigation

* [Back to 2.1 C++ Physics Engine](02-01-cpp-physics-engine.md)
* [Proceed to 2.1.2 Data Structures & Configuration Schema](02-01-02-data-structures-and-config-schema.md)
* [Explore 6. Reference Materials & Research Papers](06-reference-materials-and-research-papers.md)
