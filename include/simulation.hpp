#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <string>
#include <vector>

// Target material specification
struct Target {
    std::string name;
    double density; // kg/m^3
};

// Projectile specification (e.g., GBU-57 MOP parameters)
struct Projectile {
    std::string name;
    double length;           // meters
    double diameter;         // meters
    double total_mass;       // kg
    double explosive_mass;   // kg
    double casing_density;   // kg/m^3
    double yield_strength;   // Pascals (e.g., 2.0 GPa for Eglin steel)
};

// Scenario input definition
struct ImpactScenario {
    std::string name;
    double altitude_ft;      // feet
    double velocity;         // m/s
};

// Simulation results for a given scenario
struct SimulationResult {
    std::string scenario_name;
    double altitude_ft;
    double velocity;
    double mach_number;
    double kinetic_energy;       // Joules
    double dynamic_pressure;     // Pascals
    bool casing_failure;
    bool premature_detonation;
    double hydro_penetration;    // meters (Alekseevskii-Tate limit)
    std::string regime;
    std::string outcome_summary;
};

class ImpactSimulator {
private:
    Projectile proj;
    Target target;
    const double SPEED_OF_SOUND = 343.0; // m/s in air at sea level (approximate reference)

public:
    ImpactSimulator(const Projectile& , const Target& t);
    SimulationResult simulate(const ImpactScenario& scenario);
    void printAscii3DVisualizer(const SimulationResult& r);
    void printReport(const std::vector<SimulationResult>& results);
    void generateHtml3DVisualizer(const std::vector<SimulationResult>& results);
};

#endif // SIMULATION_HPP
