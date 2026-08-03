# JSON Data Schema Reference

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

## Target Schema (`data/targets.json`)

Each target is a named stack of one or more layers, penetrated top-to-bottom in array order.

```json
{
  "name": "Target Name",
  "layers": [
    {
      "material_name": "Layer Material",
      "thickness": 5.0,
      "density": 2650.0,
      "compressive_strength": 60.0e6,
      "rebar_volume_fraction": 0.02,
      "rebar_yield_strength": 400.0e6,
      "hugoniot_c0": 3200.0,
      "hugoniot_s": 1.9
    }
  ]
}
```

| Field | Units | Default | Description |
|---|---|---|---|
| `material_name` | - | "Unknown Material" | Display name for reports/visualizer |
| `thickness` | m | 1.0 | Layer thickness |
| `density` | kg/m^3 | 2500.0 | Layer density |
| `compressive_strength` | Pa | 60.0e6 | Quasi-static unconfined compressive strength (scaled by the CEB-FIP Dynamic Increase Factor at runtime) |
| `rebar_volume_fraction` | 0.0-1.0 | 0.0 | Steel reinforcement volume fraction |
| `rebar_yield_strength` | Pa | 0.0 | Rebar yield strength |
| `hugoniot_c0` | m/s | 3200.0 | Hugoniot bulk sound speed (Us = C0 + S*Up), used for Walker-Wasley shock impedance matching. Representative literature value, not certificate-grade. |
| `hugoniot_s` | - | 1.9 | Hugoniot slope coefficient. Representative literature value. |

Targets with no `layers` array are treated as legacy single-layer semi-infinite targets (`thickness = 100.0`, `material_name = "Legacy Material"`).

## Projectile Schema (`data/projectiles.json`)

```json
{
  "name": "Munition Name",
  "length": 6.2,
  "diameter": 0.8,
  "curvature_noseReduce": 4.8,
  "total_mass": 13600.0,
  "explosive_mass": 2400.0,
  "casing_density": 7800.0,
  "yield_strength": 2.0e9,
  "specific_heat": 460.0,
  "melting_point": 1800.0,
  "heat_of_fusion": 272000.0,
  "area_moment_inertia": 0.02,
  "elastic_modulus": 200.0e9,
  "casing_wall_thickness": 0.1,
  "hugoniot_c0": 4570.0,
  "hugoniot_s": 1.49,
  "explosive_critical_energy": 3.0e15
}
```

| Field | Units | Default | Description |
|---|---|---|---|
| `name` | - | "Unknown Projectile" | Display name |
| `length` | m | 1.0 | Overall length |
| `diameter` | m | 0.1 | Body diameter |
| `curvature_noseReduce` | m | 6.0 * diameter | Nose tip radius of curvature (used to derive CRH) |
| `total_mass` | kg | 100.0 | Total mass |
| `explosive_mass` | kg | 0.0 | Explosive fill mass (0 => kinetic rod, no explosive) |
| `casing_density` | kg/m^3 | 7800.0 | Casing/body material density |
| `yield_strength` | Pa | 1.0e9 | Casing dynamic flow stress (structural failure checks, and Y_p in the Walker-Anderson erosion model) |
| `specific_heat` | J/(kg*K) | 460.0 | Casing specific heat |
| `melting_point` | K | 1800.0 | Casing melting point |
| `heat_of_fusion` | J/kg | 272000.0 | Casing heat of fusion |
| `area_moment_inertia` | m^4 | 0.02 | Cross-section area moment of inertia (bending) |
| `elastic_modulus` | Pa | 200.0e9 | Casing Young's modulus, used for the WAPM bar wave speed c = sqrt(E/rho_p) |
| `casing_wall_thickness` | m | 0.05 | Shock transit path length from impact face to explosive fill (Walker-Wasley pulse duration estimate) |
| `hugoniot_c0` | m/s | 4570.0 | Casing Hugoniot bulk sound speed (Us = C0 + S*Up) |
| `hugoniot_s` | - | 1.49 | Casing Hugoniot slope coefficient |
| `explosive_critical_energy` | Pa^2*s | 3.0e15 | Walker-Wasley critical shock initiation energy Ec (P^2*tau >= Ec triggers detonation). Representative Comp-B-like default (calibrated so a few-hundred-m/s impact carries moderate risk, not a certainty), not explosive-certificate precision |

Note: `hugoniot_c0`/`hugoniot_s`/`explosive_critical_energy` are unused for kinetic rods (`explosive_mass == 0.0`).
