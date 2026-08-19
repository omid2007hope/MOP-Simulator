Viewed simulation.cpp:23-23

Based on a line-by-line review of `simulation.cpp`, here is the comprehensive breakdown of every single data entry (input) and calculated result (output/intermediate variable) utilized and generated in the physics engine.

### 1. Data Entries (Inputs / Provided Payloads)

These are the variables provided to the engine from the `Projectile`, `Target`, `ImpactScenario`, `Aircraft`, and `PhysicsConstants` configurations:

**Projectile Specs (`proj`)**:

- `total_mass`, `explosive_mass`, `explosive_energy_j_per_kg`
- `diameter`, `length`, `curvature_noseReduce` (CRH)
- `casing_density`, `casing_wall_thickness`, `yield_strength`
- `area_moment_inertia`, `elastic_modulus`
- `hugoniot_c0`, `hugoniot_s` (Equation of State parameters)
- `explosive_critical_energy` (Walker-Wasley initiation threshold)
- `specific_heat`, `melting_point`, `heat_of_fusion` (Thermodynamics)

**Target & Layers (`target`)**:

- `layers` array containing for each strata: `material_name`, `thickness`, `density`, `compressive_strength`, `rebar_volume_fraction`, `rebar_yield_strength`, `pulverized_depth`

**Scenario Parameters (`scenario`)**:

- `altitude_ft` (Drop altitude)
- `velocity` (Release velocity)
- `flight_path_angle`, `obliquity_angle`, `angle_of_attack`

**Universal Constants & Aircraft (`cons`, `B2_Sprit_Strategic_Bomber`)**:

- `gravity`, `PI`, `frictionFactor`, `SpeedOfSound`
- `bomber_liftCurveSlope`, `bomber_wingArea`

---

### 2. Calculated Results (Intermediate Variables)

**Atmospheric Drop Phase:**

- `atmos`: Atmospheric state (temperature, pressure, density, local speed of sound) at the current altitude using US Standard Atmosphere.
- `trim_deg` / `trim_rad`: Aerodynamic trim angle to maintain flight path before drop.
- `fpa_rad_corrected`: Flight path angle adjusted for aircraft trim.
- `current_vx`, `current_vy`, `current_velocity`: X/Y velocity vectors and absolute magnitude.
- `area`: Projectile cross-sectional area (calculated from diameter).
- `dragCoefficient` (`cd`): Mach-dependent aerodynamic drag coefficient.
- `f` / `drag_force`: Total aerodynamic drag force opposing motion.
- `guidance_pull` / `guidance_accel`: Terminal guidance corrections applied to the X-axis.
- `mach`: Current Mach number of the projectile.
- `sonic_boom_triggered`: Boolean flag activated when `current_velocity >= speed_of_sound_ms`.
- `boom_time` / `boom_alt`: Precise timestamp and altitude when the sound barrier is broken.
- `acceleration` / `g_force`: Deceleration experienced during free-fall due to air resistance.
- **Phase Outputs**: `impact_velocity` (terminal velocity at ground) and `impact_pitch` (impact angle).

**Ground Penetration Phase:**

- `target_obliquity_radians`, `obliquity_radians`: Absolute strike angle relative to the ground.
- `bar_wave_speed`: Elastic wave speed traveling through the projectile casing.
- `initial_shaft_depth`: Accumulated depth of pre-existing craters (for sequential "double-tap" strikes).
- `layer_bottom_depths`: Absolute depths where target material transitions occur.
- `critical_angle_threshold`: Deflection/Ricochet threshold (65 degrees, or 50 degrees if slow).
- `Up` (Particle velocity), `Us` (Shock wave velocity): Calculated via Hugoniot Equation of State matching.
- `P_shock`: Initial shock pressure upon striking the target surface.
- `transmitted_pressure`: Shock pressure transmitted through the casing into the explosive payload (~25% efficiency).
- `shock_energy`: Integrated shock energy flux applied to the payload.
- `dynamic_pressure`: Hydrodynamic pressure exerted on the projectile nose.
- `erosion_active`: Boolean flag triggered when `dynamic_pressure > proj.yield_strength`.
- `asymmetric_force`, `bending_moment`, `max_bending_stress`: Stresses calculated to check for structural J-Hook failure/snapping.
- `strain_rate` & `dif`: Dynamic Increase Factor scaling the concrete's strength based on strain rate.
- `effective_strength`: Final target strength resisting penetration (base strength \* DIF).
- `tunnelForce` / `axialForce`: Cavity expansion resistance slowing the projectile (Forrestal equations).
- `u`: Hydrodynamic interface erosion velocity (Alekseevskii-Tate model).
- `heat_rate`, `current_temperature`: Frictional heating and current temperature of the penetrator.
- `excess_heat`, `mass_loss`: Mass ablated when temperatures exceed `melting_point` (thermal destruction).
- `effective_linear_density`: Mass per unit length used to calculate mass loss during hydrodynamic rod erosion.
- `average_density`: Density of the target strata used to compute theoretical maximum hydrodynamic penetration depth.

---

### 3. Final SimulationResults (`res`) Populated

At the end of the simulation, these fields are finalized and returned to the visualizer:

- **Trajectory**: `altitude_ft`, `velocity`, `mach_number`, `kinetic_energy` (Joules at impact)
- **Shock & Detonation**: `shock_damage_prob_percent`, `shock_pressure_gpa_peak`, `shock_pulse_duration_us`, `premature_detonation`, `explosive_charge_survives`
- **Structural Integrity**: `casing_failure`, `erosion_occurred`, `final_rod_length`, `erosion_length_lost`
- **Penetration Performance**: `actual_penetration_depth`, `rigid_penetration`, `hydro_penetration`, `cumulative_breach_depth`, `previous_strike_depth`
- **Material Dynamics**: `dynamic_pressure`, `dynamic_increase_factor`, `bar_wave_speed`
- **Visualizer Scaling**: `explosive_mass`, `explosion_scale`, `crater_wide_radius`, `crater_narrow_radius`, `camera_shake_magnitude`, `time_scale_pen`
- **Custom Math Functions**: `kinetic_shock_joules` (from `impactShockwave`) and `total_explosive_yield` (from `explosiveShockwave`)
- **Telemetry Frames**: Two massive arrays (`drop_frames`, `penetration_frames`) containing sub-millisecond step-by-step logs of velocity, altitude, depth, G-forces, heat, and phase changes used to animate the charts.
