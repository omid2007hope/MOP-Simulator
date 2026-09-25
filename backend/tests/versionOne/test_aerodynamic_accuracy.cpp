#include <iostream>
#include <cassert>
#include <cmath>
#include <numbers>
#include "penetration/versionOne/simulation.hpp"
#include "penetration/versionOne/environment_physics.hpp"

void runTest() {
    std::cout << "[TEST] Aerodynamic Free-Fall Accuracy...\n";
    
    // Create standard structs
    Projectile p{
        .name = "Test Bomb",
        .length = 5.0,
        .diameter = 0.5,
        .curvature_noseReduce = 1.0,
        .total_mass = 10000.0,
        .explosive_mass = 2000.0,
        .casing_density = 7800.0,
        .yield_strength = 1.2e9,
        .area_moment_inertia = 0.015,
        .elastic_modulus = 210e9,
        .casing_wall_thickness = 0.04,
        .hugoniot_c0 = 4500.0,
        .hugoniot_s = 1.5,
        .explosive_critical_energy = 2e15,
        .explosive_energy_j_per_kg = 5e6,
        .specific_heat = 450.0,
        .melting_point = 1700.0,
        .heat_of_fusion = 270000.0
    };
    TargetLayer layer{
        .material_name = "Air",
        .thickness = 10.0,
        .density = 1.2,
        .compressive_strength = 0.0,
        .rebar_volume_fraction = 0.0,
        .rebar_yield_strength = 0.0,
        .pulverized_depth = 0.0,
        .hugoniot_c0 = 300.0,
        .hugoniot_s = 1.0,
        .specific_heat = 850.0,
        .melting_point = 1600.0,
        .heat_of_fusion = 350000.0
    }; // Minimal target to end simulation immediately upon hit
    Target t{"Ground", {layer}};
    Aircraft a{"Test Bomber", 150000.0, 450.0, 4.0};
    AtmosphereState atmos{300.0, 100000.0, 1.2, 340.0};
    PhysicsConstants cons;

    ImpactScenario sc{
        .name = "High Altitude Drop",
        .altitude_ft = 40000.0,
        .bombers_horizontal_velocity = 0.0,
        .velocity = 0.0,
        .flight_path_angle = 90.0,
        .obliquity_angle = 0.0,
        .angle_of_attack = 0.0
    }; // Drop from 40k ft (approx 12192 meters) straight down

    ImpactSimulator simulator(p, t, cons);
    SimulationResult res = simulator.simulate(sc, a, atmos);

    // Validate terminal parameters (Impact velocity should be very high, but constrained by terminal velocity)
    // Terminal velocity v_t = sqrt(2 * m * g / (rho * A * Cd))
    double A = std::numbers::pi * std::pow(p.diameter / 2.0, 2);
    double rho_sea_level = EnvironmentPhysics::standardAtmosphere(0.0, cons).density_kgm3;
    
    // Free fall time from 12192m without drag is ~50s. Final velocity without drag would be ~490m/s.
    // With drag, it should be slightly less.
    assert(res.impact_velocity > 400.0);
    assert(res.impact_velocity < 500.0);

    // Free fall time from 12192m without drag is ~50s. With drag, it should be longer.
    if (!res.drop_frames.empty()) {
        assert(res.drop_frames.back().time > 49.0);
    }

    // Validate Mach number
    double expected_mach = res.impact_velocity / EnvironmentPhysics::standardAtmosphere(0.0, cons).speed_of_sound_ms;
    assert(std::abs(res.mach_number - expected_mach) < 1e-3);

    std::cout << "[TEST] Aerodynamic Free-Fall Accuracy PASSED!\n";
}

int main() {
    runTest();
    return 0;
}
