// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <numbers>
#include <string>
#include <vector>

struct PhysicsConstants
{
    const double SPEED_OF_SOUND = 343.0;
    const double PI = std::numbers::pi;

    // Engine behavior constants
    const double shockDamageMultiplier = 85.0;
    const double shockDamageExponent = 1.5;
    const double frictionFactor = 0.1;
};

// Target layer specification
struct TargetLayer
{
    std::string material_name;
    double thickness;             // meters
    double density;               // kg/m^3
    double compressiveStrength;   // Pascals
    double rebar_volume_fraction; // 0.0 to 1.0 (e.g. 0.02 for heavily reinforced)
    double rebar_yield_strength;  // Pascals
};

// Target material specification
struct Target
{
    std::string name;
    std::vector<TargetLayer> layers;
};

// Projectile specification (e.g., GBU-57 MOP parameters)
struct Projectile
{
    std::string name;
    double length;               // meters
    double diameter;             // meters
    double curvature_noseReduce; // meters
    double total_mass;           // kg
    double explosive_mass;       // kg
    double casing_density;       // kg/m^3
    double yield_strength;       // Pascals (e.g., 2.0 GPa for Eglin steel)

    // Thermal ablation properties
    double specific_heat;  // J/(kg*K)
    double melting_point;  // Kelvin
    double heat_of_fusion; // J/kg

    // Structural properties
    double area_moment_inertia; // m^4 (for bending moment calculations)
};

// Scenario input definition
struct ImpactScenario
{
    std::string name;
    double altitude_ft;     // feet
    double velocity;        // m/s
    double obliquity_angle; // Degrees (0 = perfectly perpendicular)
    double angle_of_attack; // Degrees
};

// Simulation results for a given scenario
struct SimulationResult
{
    std::string scenario_name;
    double altitude_ft;
    double velocity;
    double mach_number;
    double kinetic_energy;   // Joules
    double dynamic_pressure; // Pascals
    bool casing_failure;
    bool premature_detonation;
    double hydro_penetration;         // meters (Alekseevskii-Tate limit)
    double rigid_penetration;         // meters (Work-energy concrete deceleration model)
    double actual_penetration_depth;  // meters (Selected depth based on regime)
    double shock_damage_prob_percent; // 0% to 100% chance of explosive failure from shock
    bool explosive_charge_survives;   // true if charge holds intact without shock damage
    bool is_kinetic_rod;              // true if explosive_mass == 0 or yield_strength == 0
    std::string regime;
    std::string outcome_summary;
};

class ImpactSimulator
{
private:
    Projectile proj;
    Target target;
    PhysicsConstants cons;

public:
    ImpactSimulator(const Projectile& p, const Target& t, const PhysicsConstants& c);
    SimulationResult simulate(const ImpactScenario& scenario);
    void printAscii3DVisualizer(const SimulationResult& r);
    void printReport(const std::vector<SimulationResult>& results);
    void generateHtml3DVisualizer(const std::vector<SimulationResult>& results,
                                  const std::string& basePath);
};

#endif // SIMULATION_HPP
