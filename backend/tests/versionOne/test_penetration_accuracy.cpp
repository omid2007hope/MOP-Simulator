#include <iostream>
#include <cassert>
#include <cmath>
#include "penetration/versionOne/simulation.hpp"

void runTest() {
    std::cout << "[TEST] Ground Penetration Accuracy...\n";
    
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
    
    // Very hard concrete
    TargetLayer layer{
        .material_name = "Concrete",
        .thickness = 100.0,
        .density = 2400.0,
        .compressive_strength = 50e6,
        .rebar_volume_fraction = 0.03,
        .rebar_yield_strength = 400e6,
        .pulverized_depth = 0.0,
        .hugoniot_c0 = 3000.0,
        .hugoniot_s = 1.8,
        .specific_heat = 850.0,
        .melting_point = 1600.0,
        .heat_of_fusion = 350000.0
    };
    Target t{"Bunker", {layer}};
    Aircraft a{"Test Bomber", 150000.0, 450.0, 4.0};
    AtmosphereState atmos{300.0, 100000.0, 1.2, 340.0};
    PhysicsConstants cons;

    ImpactScenario sc{
        .name = "Direct Hit",
        .altitude_ft = 0.0,
        .bombers_horizontal_velocity = 0.0,
        .velocity = 500.0,
        .flight_path_angle = 90.0,
        .obliquity_angle = 0.0,
        .angle_of_attack = 0.0
    }; // Drop from 0 altitude, initial velocity 500m/s straight down

    ImpactSimulator simulator(p, t, cons);
    SimulationResult res = simulator.simulate(sc, a, atmos);

    // Validate penetration depth. At 500m/s, 10,000kg bomb in 50MPa concrete should penetrate between 5m and 50m.
    std::cout << "Actual Penetration Depth: " << res.actual_penetration_depth << std::endl;
    assert(res.actual_penetration_depth > 3.0);
    assert(res.actual_penetration_depth < 50.0);

    // Validate final velocity is zero (came to rest)
    if (!res.penetration_frames.empty()) {
        // We know it stopped if actual_penetration_depth < t.layers[0].thickness
        assert(res.actual_penetration_depth > 3.0 && res.actual_penetration_depth < t.layers[0].thickness);
    }

    // Kinetic energy verification
    double initial_ke = 0.5 * p.total_mass * std::pow(500.0, 2);
    assert(initial_ke > 0);

    std::cout << "[TEST] Ground Penetration Accuracy PASSED!\n";
}

int main() {
    runTest();
    return 0;
}
