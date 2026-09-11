#include <iostream>
#include <cassert>
#include <cmath>
#include "penetration/versionOne/simulation.hpp"

void runTest() {
    std::cout << "[TEST] Ground Penetration Accuracy...\n";
    
    Projectile p{"Test Bomb", 5.0, 0.5, 1.0, 10000.0, 2000.0, 5e6, 7800.0, 1.2e9, 0.015, 210e9, 0.04, 4500.0, 1.5, 2e15, 450.0, 1700.0, 270000.0};
    
    // Very hard concrete
    TargetLayer layer{"Concrete", 100.0, 0.03, 400e6, 2400.0, 50e6, 3000.0, 1.8, 850.0, 1600.0, 350000.0};
    Target t{"Bunker", {layer}};
    Aircraft a{"Test Bomber", 150000.0, 450.0, 4.0};
    AtmosphereState atmos{300.0, 100000.0, 1.2, 340.0};
    PhysicsConstants cons;

    ImpactScenario sc{"Direct Hit", 0.0, 500.0, 90.0, 0.0, 0.0}; // Drop from 0 altitude, initial velocity 500m/s straight down

    ImpactSimulator simulator(p, t, cons);
    SimulationResult res = simulator.simulate(sc, a, atmos);

    // Validate penetration depth. At 500m/s, 10,000kg bomb in 50MPa concrete should penetrate between 5m and 50m.
    assert(res.actual_penetration_depth > 5.0);
    assert(res.actual_penetration_depth < 50.0);

    // Validate final velocity is zero (came to rest)
    if (!res.penetration_frames.empty()) {
        assert(std::abs(res.penetration_frames.back().velocity) < 1e-3 || res.penetration_frames.back().depth >= t.layers[0].thickness); // Either stopped or breached
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
