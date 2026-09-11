#include <iostream>
#include <chrono>
#include <vector>
#include <cassert>
#include "penetration/versionOne/simulation.hpp"

void runTest() {
    std::cout << "[TEST] Stress & Performance Test...\n";
    
    Projectile p{"Test Bomb", 5.0, 0.5, 1.0, 10000.0, 2000.0, 5e6, 7800.0, 1.2e9, 0.015, 210e9, 0.04, 4500.0, 1.5, 2e15, 450.0, 1700.0, 270000.0};
    TargetLayer layer{"Concrete", 10.0, 0.03, 400e6, 2400.0, 50e6, 3000.0, 1.8, 850.0, 1600.0, 350000.0};
    Target t{"Bunker", {layer}};
    Aircraft a{"Test Bomber", 150000.0, 450.0, 4.0};
    AtmosphereState atmos{300.0, 100000.0, 1.2, 340.0};
    PhysicsConstants cons;

    ImpactSimulator simulator(p, t, cons);
    std::vector<SimulationResult> results;

    const int numScenarios = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numScenarios; ++i) {
        ImpactScenario sc{"Stress Test", 30000.0, 0.0, 90.0, 0.0, 0.0};
        results.push_back(simulator.simulate(sc, a, atmos));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    
    std::cout << "Executed " << numScenarios << " simulations in " << diff.count() << " seconds.\n";
    
    // Performance assertion: Ensure it takes less than 5 seconds for 1000 scenarios
    assert(diff.count() < 5.0);
    assert(results.size() == numScenarios);

    std::cout << "[TEST] Stress & Performance Test PASSED!\n";
}

int main() {
    runTest();
    return 0;
}
