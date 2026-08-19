# 6.1 Physics Reference Documents

// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

---

## 🧪 Thermophysical Properties of Elements & Engineering Alloys

The simulation engine references validated thermophysical constants for casing metallurgy, explosive binders, and geological minerals:

| Material / Element | Density ($\text{g/cm}^3$) | Melting Point ($\text{K}$) | Boiling Point ($\text{K}$) | Specific Heat ($\text{J/(kg}\cdot\text{K)}$) |
| :--- | :---: | :---: | :---: | :---: |
| **Water @STP** | $1.000$ | $273.15$ | $373.15$ | $4,184$ |
| **Beryllium** | $1.850$ | $1,560.00$ | $2,742.00$ | $1,825$ |
| **Carbon (Graphite)** | $2.267$ | — <sup>[1]</sup> | $4,000.00$ <sup>[1]</sup> | $710$ |
| **Aluminium (7075-T6)**| $2.700$ | $933.47$ | $2,792.00$ | $897$ |
| **Titanium (Ti-6Al-4V)**| $4.500$ | $1,941.00$ | $3,560.00$ | $526$ |
| **Iron / Structural Steel** | $7.850$ | $1,811.00$ | $3,134.00$ | $460$ |
| **Eglin Steel (ES-1)**| $7.850$ | $1,780.00$ | $3,100.00$ | $470$ |
| **AF1410 Alloy Steel** | $7.870$ | $1,795.00$ | $3,110.00$ | $465$ |
| **Nickel** | $8.912$ | $1,728.00$ | $3,186.00$ | $444$ |
| **Copper** | $8.960$ | $1,357.77$ | $2,835.00$ | $385$ |
| **Tungsten Alloy (93W)**| $17.600$ | $3,695.00$ | $5,828.00$ | $134$ |
| **Depleted Uranium** | $19.100$ | $1,408.00$ | $4,404.00$ | $116$ |
| **Platinum** | $21.460$ | $2,041.40$ | $4,098.00$ | $133$ |
| **Lead** | $11.342$ | $600.61$ | $2,022.00$ | $129$ |

> [!NOTE]
> **[1] Carbon Sublimation**: Under standard atmospheric pressure ($1\text{ atm}$), carbon does not transition through a liquid phase; it sublimes directly from solid to vapor at approximately $4,100\text{ K}$.

---

## 💥 Casing Material Yield & Failure Thresholds

1. **Stagnation Dynamic Pressure Yield (Crush Failure)**:
   Occurs when hydrodynamic nose stagnation pressure exceeds dynamic yield strength:
   $$P_{dyn} = \frac{1}{2} \rho_t v^2 > Y_p$$

2. **Alekseevskii-Tate Hydrodynamic Penetration Limit**:
   At hypervelocity speeds ($v > 1200\text{ m/s}$), penetration depth approaches the theoretical density-ratio limit for completely eroded penetrators:
   $$L_{max} = L_0 \sqrt{\frac{\rho_p}{\rho_t}}$$

3. **Structural Bending Moment Failure (J-Hooking)**:
   During oblique entry, asymmetric normal forces produce a bending moment $M_b$. If maximum outer fiber bending stress exceeds casing yield stress ($\sigma_b > \sigma_{yield}$), catastrophic snapping occurs:
   $$\sigma_b = \frac{M_b \cdot (D/2)}{I_{area}} > \sigma_{yield}$$

---

## 🔥 Frictional Thermodynamics & Mass Ablation

Frictional work converted to thermal energy along the casing boundary:

$$\dot{Q}_{fric} = \mu_{fric} \cdot F_{tunnel} \cdot v$$

$$\Delta T = \int \frac{\dot{Q}_{fric}}{m(t) \cdot c_p} dt$$

When temperature reaches $T_{melt}$, casing mass ablates governed by latent heat of fusion ($L_f = 272,000\text{ J/kg}$ for high-strength steel alloys):

$$\frac{dm_{ablate}}{dt} = \frac{\dot{Q}_{excess}}{L_f}$$

---

## 🧭 Navigation

* [Back to 6. Reference Materials & Research Papers](06-reference-materials-and-research-papers.md)
* [Proceed to 6.2 Ballistic Drag Model References](06-02-ballistic-drag-model-references.md)
* [Explore 2.1.1 Physics Models & Numerical Methods](02-01-01-physics-models-and-numerical-methods.md)
