Physics Engine Upgrade: Time-Dependent Dynamics & Complex Materials
This plan addresses the identified gaps in the current MOP simulator by transitioning from a single-step, closed-form rigid body equation to a robust, time-integrated continuum mechanics approach.

User Review Required
CAUTION

Moving to a time-stepped simulation changes the fundamental architecture of the application. The simulator will no longer simply output a final depth using a single formula; it will calculate the state of the projectile at thousands of microsecond intervals.

Do you want the time-history data (e.g., velocity vs. time, depth vs. time) exported to a CSV or embedded into the HTML 3D visualizer?
How should we handle existing JSON configurations for targets, given they will now require layers and rebar fractions? Should I write a migration script or just update them manually?
Open Questions
WARNING

Ablation Model: Do we have a specific thermal ablation coefficient for the projectile casing material (e.g., Eglin Steel), or should we use a standard generalized mass-loss empirical formula?
Bending Moments: Should the simulation immediately terminate (declare structural failure) if the lateral bending moment exceeds the yield strength, or should we model J-hooking/deflection?
Proposed Changes
Data Structures & Configuration Layer
We need to update our structures in include/simulation.hpp to support the new physics.

[MODIFY]
simulation.hpp
Target Layers: Replace the monolithic Target with a structure containing multiple layers.
cpp

struct TargetLayer {
std::string material_name;
double thickness; // meters
double density;
double compressiveStrength;
double rebar_volume_fraction; // 0.0 to 1.0
double rebar_yield_strength;
};
struct Target {
std::string name;
std::vector<TargetLayer> layers;
};
Projectile Dynamics: Add ablation and bending properties.
cpp

struct Projectile {
// ... existing properties ...
double specific_heat; // J/(kg\*K) for thermal ablation
double melting_point; // Kelvin
double area_moment_inertia; // For bending moment calculations
};
Scenario Enhancements: Include Obliquity and AoA.
cpp

struct ImpactScenario {
// ... existing properties ...
double obliquity_angle; // Degrees (0 is perfectly perpendicular)
double angle_of_attack; // Degrees
};
Simulation Engine
The core logic in src/simulation.cpp must be completely rewritten to accommodate a time loop (dt).

[MODIFY]
simulation.cpp
Time-Integration Loop: Replace the closed-form calculation with a while loop (while (velocity > 0 && !failed)) using a small dt (e.g., 1e-5 seconds).
Heterogeneous Targets: During the loop, track the current depth and determine which TargetLayer the nose of the projectile is currently in. Apply the density and strength of that specific layer.
Rebar and Aggregate Interlock: Modify the dynamic pressure/deceleration equation to include a term for steel reinforcement tensile strength, scaled by the rebar*volume_fraction.
Mass Loss (Ablation): Calculate frictional heating at each time step. If the temperature exceeds the melting_point, subtract from total_mass and adjust the diameter/CRH dynamically.
Obliquity and AoA (Bending Moments): Calculate the asymmetric force vector based on the attack angle and obliquity. Compute the lateral bending moment ($M = F*{lateral} \times L\_{effective}$). If the stress ($\sigma = M \cdot y / I$) exceeds yield_strength, trigger a casing_failure.
Configuration Loaders and Defaults
[MODIFY]
config_loader.hpp
Update loadTargets to parse a JSON array of layers for each target.
[MODIFY]
config_loader.cpp
Implement the parsing logic for TargetLayer arrays from the JSON input.
[MODIFY]
default.hpp
Update the CONCRETE_DEFAULT and MOP_DEFAULT constants to include the new required parameters (e.g., layering, rebar, thermal properties).
Verification Plan
Automated Tests
If there are existing unit tests, update them to use the new nested Target structure and check that a single-layer target closely matches the old closed-form model results (as a baseline regression test).
Manual Verification
Compile and run the simulator with varying Obliquity and AoA to ensure the projectile fails (snaps) under high angles.
Create a target with a 1m soil layer, 2m heavily reinforced concrete layer, and 10m rock layer, and verify the deceleration curve drops sharply when hitting the concrete.
Check the generated HTML 3D visualizer to ensure the new physics properties are displayed correctly in the UI.
