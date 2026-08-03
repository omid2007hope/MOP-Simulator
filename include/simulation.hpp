// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <numbers>
#include <string>
#include <vector>

struct PhysicsConstants
{
    const double gravity = 9.81;
    const double SPEED_OF_SOUND = 343.0;
    const double PI = std::numbers::pi;

    // Engine behavior constants
    const double shockDamageMultiplier = 85.0;
    const double shockDamageExponent = 1.5;
    const double frictionFactor = 0.1;
};

struct AltitudeDensityPoint
{
    double altitude_ft = 0.0; // Altitude in feet
    double density = 1.225;   // Air density in kg/m^3
};

struct AirLayers
{
    std::vector<AltitudeDensityPoint> eachLayer = {{0.0, 1.2250},
                                                   {10000.0, 0.9041},
                                                   {20000.0, 0.6531},
                                                   {30000.0, 0.4581},
                                                   {40000.0, 0.3119},
                                                   {50000.0, 0.2031},
                                                   {60000.0, 0.1268},
                                                   {70000.0, 0.0765},
                                                   {80000.0, 0.0457}};
};

// Target layer specification
struct TargetLayer
{
    std::string material_name;
    double thickness = 1.0;               // meters
    double density = 2500.0;              // kg/m^3
    double compressive_strength = 60.0e6; // Pascals
    double rebar_volume_fraction = 0.0;   // 0.0 to 1.0 (e.g. 0.02 for heavily reinforced)
    double rebar_yield_strength = 0.0;    // Pascals
    double pulverized_depth = 0.0;        // meters (depth of crater from previous strikes)
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
    double length = 1.0;               // meters
    double diameter = 0.1;             // meters
    double curvature_noseReduce = 0.6; // meters
    double total_mass = 100.0;         // kg
    double explosive_mass = 0.0;       // kg
    double casing_density = 7800.0;    // kg/m^3
    double yield_strength = 1.0e9;     // Pascals (e.g., 2.0 GPa for Eglin steel)

    // Thermal ablation properties
    double specific_heat = 460.0;     // J/(kg*K)
    double melting_point = 1800.0;    // Kelvin
    double heat_of_fusion = 272000.0; // J/kg

    // Structural properties
    double area_moment_inertia = 0.02; // m^4 (for bending moment calculations)
};

// Scenario input definition
struct ImpactScenario
{
    std::string name;
    double altitude_ft = 0.0;     // feet
    double velocity = 0.0;        // m/s
    double obliquity_angle = 0.0; // Degrees (0 = perfectly perpendicular)
    double angle_of_attack = 0.0; // Degrees
};

struct TelemetryFrame
{
    double time = 0.0;
    double altitude = 0.0;
    double depth = 0.0;
    double velocity = 0.0;
    double mach = 0.0;
    double dynamic_pressure = 0.0;
    bool is_sonic_boom = false;
};

// Simulation results for a given scenario
struct SimulationResult
{
    std::string scenario_name;
    double altitude_ft = 0.0;
    double velocity = 0.0;
    double mach_number = 0.0;
    double kinetic_energy = 0.0;   // Joules
    double dynamic_pressure = 0.0; // Pascals
    bool casing_failure = false;
    bool premature_detonation = false;
    double hydro_penetration = 0.0;         // meters (Alekseevskii-Tate limit)
    double rigid_penetration = 0.0;         // meters (Work-energy concrete deceleration model)
    double actual_penetration_depth = 0.0;  // meters (Selected depth based on regime)
    double shock_damage_prob_percent = 0.0; // 0% to 100% chance of explosive failure from shock
    bool explosive_charge_survives = true;  // true if charge holds intact without shock damage
    bool is_kinetic_rod = false;            // true if explosive_mass == 0 or yield_strength == 0
    std::string regime;
    std::string outcome_summary;

    std::vector<TelemetryFrame> drop_frames;
    std::vector<TelemetryFrame> penetration_frames;
};

class ImpactSimulator
{
private:
    Projectile proj;
    Target target;
    PhysicsConstants cons;

    double getMachDependentDrag(double mach, double baseCd) const;

public:
    ImpactSimulator(const Projectile& p, const Target& t, const PhysicsConstants& c);
    SimulationResult simulate(const ImpactScenario& scenario);
    void printAscii3DVisualizer(const SimulationResult& r);
    void printReport(const std::vector<SimulationResult>& results);
    void generateHtml3DVisualizer(const std::vector<SimulationResult>& results,
                                  const std::string& basePath);
};

#endif // SIMULATION_HPP
