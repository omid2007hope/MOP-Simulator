#include <iostream>
#include <cassert>
#include <cmath>
#include "penetration/versionOne/simulation.hpp"

void runTest() {
    std::cout << "[TEST] Scenario Duplication Determinism...\n";
    
    // Create structs
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
        .material_name = "Concrete",
        .thickness = 10.0,
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
    Target t{"Test Bunker", {layer}};
    Aircraft a{"Test Bomber", 150000.0, 450.0, 4.0};
    AtmosphereState atmos{300.0, 100000.0, 1.2, 340.0};
    PhysicsConstants cons;

    ImpactScenario sc1{
        .name = "Test Scenario 1",
        .altitude_ft = 30000.0,
        .bombers_horizontal_velocity = 0.0,
        .velocity = 0.0,
        .flight_path_angle = 90.0,
        .obliquity_angle = 0.0,
        .angle_of_attack = 0.0
    };
    ImpactScenario sc2{
        .name = "Test Scenario 2",
        .altitude_ft = 30000.0,
        .bombers_horizontal_velocity = 0.0,
        .velocity = 0.0,
        .flight_path_angle = 90.0,
        .obliquity_angle = 0.0,
        .angle_of_attack = 0.0
    };

    ImpactSimulator simulator1(p, t, cons);
    
    // Run scenario 1
    SimulationResult res1 = simulator1.simulate(sc1, a, atmos);
    
    // Create new simulator for scenario 2 to ensure fresh target state
    ImpactSimulator simulator2(p, t, cons);
    
    // Run scenario 2
    SimulationResult res2 = simulator2.simulate(sc2, a, atmos);

    // Verify exactly identical determinism
    std::cout << "res1 depth: " << res1.actual_penetration_depth << std::endl;
    std::cout << "res2 depth: " << res2.actual_penetration_depth << std::endl;
    assert(std::abs(res1.actual_penetration_depth - res2.actual_penetration_depth) < 1e-9);
    assert(std::abs(res1.impact_velocity - res2.impact_velocity) < 1e-9);
    assert(std::abs(res1.shock_pressure_gpa_peak - res2.shock_pressure_gpa_peak) < 1e-9);
    assert(res1.casing_failure == res2.casing_failure);

    // Verify trace arrays are identical in length
    assert(res1.penetration_frames.size() == res2.penetration_frames.size());
    
    for (size_t i = 0; i < res1.penetration_frames.size(); ++i) {
        assert(std::abs(res1.penetration_frames[i].time - res2.penetration_frames[i].time) < 1e-9);
        assert(std::abs(res1.penetration_frames[i].depth - res2.penetration_frames[i].depth) < 1e-9);
    }

    std::cout << "[TEST] Scenario Duplication Determinism PASSED!\n";
}

int main() {
    runTest();
    return 0;
}
