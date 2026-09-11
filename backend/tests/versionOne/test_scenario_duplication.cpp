#include <iostream>
#include <cassert>
#include <cmath>
#include "penetration/versionOne/simulation.hpp"

void runTest() {
    std::cout << "[TEST] Scenario Duplication Determinism...\n";
    
    // Create structs
    Projectile p{"Test Bomb", 5.0, 0.5, 1.0, 10000.0, 2000.0, 5e6, 7800.0, 1.2e9, 0.015, 210e9, 0.04, 4500.0, 1.5, 2e15, 450.0, 1700.0, 270000.0};
    TargetLayer layer{"Concrete", 10.0, 0.03, 400e6, 2400.0, 50e6, 3000.0, 1.8, 850.0, 1600.0, 350000.0};
    Target t{"Test Bunker", {layer}};
    Aircraft a{"Test Bomber", 150000.0, 450.0, 4.0};
    AtmosphereState atmos{300.0, 100000.0, 1.2, 340.0};
    PhysicsConstants cons;

    ImpactScenario sc1{"Test Scenario 1", 30000.0, 0.0, 90.0, 0.0, 0.0};
    ImpactScenario sc2{"Test Scenario 2", 30000.0, 0.0, 90.0, 0.0, 0.0};

    ImpactSimulator simulator(p, t, cons);
    
    // Run scenario 1
    SimulationResult res1 = simulator.simulate(sc1, a, atmos);
    
    // Run scenario 2
    SimulationResult res2 = simulator.simulate(sc2, a, atmos);

    // Verify exactly identical determinism
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
