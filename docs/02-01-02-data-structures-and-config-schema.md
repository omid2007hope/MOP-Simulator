# 2.1.2 Data Structures & Configuration Schema

// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

---

## 🧱 C++ Core Data Structures (`include/penetration/simulation.hpp`)

The physical data types are strongly typed, memory-compact C++23 structures.

### 1. `PhysicsConstants`
Universal physical and atmospheric constants used during numerical integration.

```cpp
struct PhysicsConstants {
    const double gravity = 9.81;                  // m/s^2
    const double PI = std::numbers::pi;           // 3.141592653589793
    const double frictionFactor = 0.1;            // Dimensionless Coulomb friction
    const double universalGasConstant = 8.31432;  // J/(mol*K)
    const double molarMassAir = 0.0289644;        // kg/mol
    const double adiabaticIndexAir = 1.4;         // Gamma for air
    const double earthRadius = 6356766.0;         // m
    const double SpeedOfSound = 343.0;            // Reference m/s
};
```

---

### 2. `Projectile`
Defines geometric dimensions, metallurgical casing properties, thermodynamic constants, and explosive fill parameters.

```cpp
struct Projectile {
    std::string name;
    double length = 0.0;                       // Total munition length (m)
    double diameter = 0.0;                     // Body outer diameter (m)
    double curvature_noseReduce = 0.0;         // Nose radius of curvature (m) -> derives CRH
    double total_mass = 0.0;                   // Gross all-up mass (kg)
    double explosive_mass = 0.0;               // High-explosive fill mass (kg) (0.0 = kinetic rod)
    double casing_density = 0.0;               // Casing metal density (kg/m^3) (e.g. 7800 for steel)
    double yield_strength = 0.0;               // Dynamic yield stress Y_p (Pa)
    double area_moment_inertia = 0.0;          // Bending moment of inertia (m^4)
    double elastic_modulus = 0.0;              // Young's modulus E (Pa) -> derives bar wave speed
    double casing_wall_thickness = 0.0;        // Thickness of casing wall (m)
    double hugoniot_c0 = 0.0;                  // Hugoniot bulk sound speed C_0 (m/s)
    double hugoniot_s = 0.0;                   // Hugoniot slope coefficient S
    double explosive_critical_energy = 0.0;    // Walker-Wasley initiation threshold E_c (Pa^2*s)
    double explosive_energy_j_per_kg = 0.0;    // Specific explosive yield (J/kg) (e.g. 5.2e6 for AFX-757)
    double specific_heat = 460.0;              // Heat capacity c_p (J/(kg*K))
    double melting_point = 1800.0;             // Melting temperature (K)
    double heat_of_fusion = 272000.0;          // Latent heat of fusion L_f (J/kg)
};
```

---

### 3. `TargetLayer` & `Target`
Represents stratified geological layers or multi-strata reinforced bunkers.

```cpp
struct TargetLayer {
    std::string material_name;                 // Material display name
    double thickness = 0.0;                    // Layer depth thickness (m)
    double density = 0.0;                      // Material density (kg/m^3)
    double compressive_strength = 0.0;         // Static unconfined compressive strength f_c' (Pa)
    double rebar_volume_fraction = 0.0;        // Volumetric reinforcement fraction (0.0 - 1.0)
    double rebar_yield_strength = 0.0;         // Rebar steel yield stress (Pa)
    double pulverized_depth = 0.0;             // Cumulative depth degraded by prior strikes (m)
    double hugoniot_c0 = 0.0;                  // Target Hugoniot bulk sound speed (m/s)
    double hugoniot_s = 0.0;                   // Target Hugoniot slope parameter
};

struct Target {
    std::string name;
    std::vector<TargetLayer> layers;
    
    void pulverizeDepth(double breachDepth);   // Weakens layers for sequential salvo runs
};
```

---

### 4. `ImpactScenario`
Kinematic release and drop parameters.

```cpp
struct ImpactScenario {
    std::string name;                          // Scenario description
    double altitude_ft = 0.0;                  // Release altitude (feet)
    double velocity = 0.0;                     // Aircraft release speed (m/s)
    double flight_path_angle = 0.0;            // Release pitch angle (degrees)
    double obliquity_angle = 0.0;              // Surface strike obliquity (degrees from normal)
    double angle_of_attack = 0.0;              // Angle of attack relative to velocity vector (degrees)
};
```

---

### 5. `TelemetryFrame`
High-frequency sub-millisecond recording emitted at every integration time-step.

```cpp
struct TelemetryFrame {
    double time = 0.0;                         // Simulation elapsed time (s)
    double altitude = 0.0;                     // Altitude above ground (m)
    double depth = 0.0;                        // Depth below ground surface (m)
    double velocity = 0.0;                     // Absolute velocity (m/s)
    double mach = 0.0;                         // Instantaneous Mach number
    double dynamic_pressure = 0.0;             // Stagnation dynamic pressure (Pa)
    bool is_sonic_boom = false;                // Triggered at sound barrier transition
    double heat = 0.0;                         // Casing temperature (K)
    double g_force = 0.0;                      // Deceleration in g's
    double pitch_rad = 0.0;                    // Pitch orientation (radians)
    bool is_eroding = false;                   // True if undergoing WAPM plastic erosion
    double dif = 0.0;                          // Dynamic Increase Factor applied
    double remaining_length = 0.0;             // Current projectile length (m)
    double obliquity_deg = 0.0;                // Instantaneous obliquity (degrees)
    // Detailed vector & force components
    double current_vx = 0.0, current_vy = 0.0;
    double drag_coefficient = 0.0, drag_force = 0.0;
    double guidance_pull = 0.0;
    double Up = 0.0, Us = 0.0, P_shock = 0.0;
    double transmitted_pressure = 0.0, shock_energy = 0.0;
    double asymmetric_force = 0.0, bending_moment = 0.0, max_bending_stress = 0.0;
    double strain_rate = 0.0, effective_strength = 0.0, tunnel_force = 0.0;
    double interface_erosion_velocity = 0.0, heat_rate = 0.0;
    double excess_heat = 0.0, mass_loss = 0.0;
};
```

---

### 6. `SimulationResult`
Aggregated final state returned to callers and persisted to MongoDB.

```cpp
struct SimulationResult {
    std::string scenario_name;
    double altitude_ft = 0.0;
    double velocity = 0.0;
    double mach_number = 0.0;
    double kinetic_energy = 0.0;               // Impact kinetic energy (Joules)
    double dynamic_pressure = 0.0;             // Peak dynamic pressure (Pa)
    bool casing_failure = false;               // True if structural yield or snap occurred
    bool premature_detonation = false;         // True if Walker-Wasley threshold exceeded
    double actual_penetration_depth = 0.0;     // Final breach depth (m)
    double rigid_penetration = 0.0;            // Depth accumulated in rigid regime (m)
    double hydro_penetration = 0.0;            // Depth accumulated in hydrodynamic regime (m)
    double shock_damage_prob_percent = 0.0;    // Probability of explosive deflagration
    bool explosive_charge_survives = true;
    bool is_kinetic_rod = false;
    std::string regime;                        // Primary terminal regime label
    std::string outcome_summary;
    bool erosion_occurred = false;
    double final_rod_length = 0.0;
    double erosion_length_lost = 0.0;
    double dynamic_increase_factor = 0.0;
    double bar_wave_speed = 0.0;
    double shock_pressure_gpa_peak = 0.0;
    double shock_pulse_duration_us = 0.0;
    double cumulative_breach_depth = 0.0;
    std::vector<TelemetryFrame> drop_frames;
    std::vector<TelemetryFrame> penetration_frames;
};
```

---

## 📄 JSON Configuration Schemas

### Target Schema (`data/targets.json`)

```json
{
  "name": "Heavy Reinforced Concrete Bunker",
  "layers": [
    {
      "material_name": "Overburden Soil",
      "thickness": 3.0,
      "density": 1900.0,
      "compressive_strength": 5.0e6,
      "rebar_volume_fraction": 0.0,
      "rebar_yield_strength": 0.0,
      "hugoniot_c0": 1800.0,
      "hugoniot_s": 1.6
    },
    {
      "material_name": "70MPa Ultra-High Performance Concrete",
      "thickness": 8.0,
      "density": 2650.0,
      "compressive_strength": 70.0e6,
      "rebar_volume_fraction": 0.03,
      "rebar_yield_strength": 500.0e6,
      "hugoniot_c0": 3200.0,
      "hugoniot_s": 1.9
    }
  ]
}
```

---

### Projectile Schema (`data/projectiles.json`)

```json
{
  "name": "GBU-57A/B Massive Ordnance Penetrator (MOP)",
  "length": 6.2,
  "diameter": 0.8,
  "curvature_noseReduce": 4.8,
  "total_mass": 13608.0,
  "explosive_mass": 2404.0,
  "casing_density": 7850.0,
  "yield_strength": 2.1e9,
  "specific_heat": 460.0,
  "melting_point": 1800.0,
  "heat_of_fusion": 272000.0,
  "area_moment_inertia": 0.0201,
  "elastic_modulus": 210.0e9,
  "casing_wall_thickness": 0.09,
  "hugoniot_c0": 4570.0,
  "hugoniot_s": 1.49,
  "explosive_critical_energy": 3.0e15,
  "explosive_energy_j_per_kg": 5.2e6
}
```

---

## 🧭 Navigation

* [Back to 2.1 C++ Physics Engine](02-01-cpp-physics-engine.md)
* [Proceed to 2.2 Node.js Automation Layer](02-02-nodejs-automation-layer.md)
* [Explore 2.2.3 MongoDB Data Models](02-02-03-mongodb-data-models.md)
