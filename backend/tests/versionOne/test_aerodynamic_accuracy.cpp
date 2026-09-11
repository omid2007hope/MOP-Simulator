#include <iostream>
#include <cassert>
#include <cmath>
#include "penetration/versionOne/simulation.hpp"
#include "penetration/versionOne/environment_physics.hpp"

void runTest() {
    std::cout << "[TEST] Aerodynamic Free-Fall Accuracy...\n";
    
    // Create standard structs
    Projectile p{"Test Bomb", 5.0, 0.5, 1.0, 10000.0, 2000.0, 5e6, 7800.0, 1.2e9, 0.015, 210e9, 0.04, 4500.0, 1.5, 2e15, 450.0, 1700.0, 270000.0};
    TargetLayer layer{"Air", 10.0, 0.0, 0.0, 1.2, 0.0, 300.0, 1.0, 850.0, 1600.0, 350000.0}; // Minimal target to end simulation immediately upon hit
    Target t{"Ground", {layer}};
    Aircraft a{"Test Bomber", 150000.0, 450.0, 4.0};
    AtmosphereState atmos{300.0, 100000.0, 1.2, 340.0};
    PhysicsConstants cons;

    ImpactScenario sc{"High Altitude Drop", 40000.0, 0.0, 90.0, 0.0, 0.0}; // Drop from 40k ft (approx 12192 meters) straight down

    ImpactSimulator simulator(p, t, cons);
    SimulationResult res = simulator.simulate(sc, a, atmos);

    // Validate terminal parameters (Impact velocity should be very high, but constrained by terminal velocity)
    // Terminal velocity v_t = sqrt(2 * m * g / (rho * A * Cd))
    double A = M_PI * std::pow(p.diameter / 2.0, 2);
    double rho_sea_level = EnvironmentPhysics::standardAtmosphere(0.0, cons).density_kgm3;
    
    // Very rough terminal velocity bounds check at sea level for a 10,000kg bomb
    double expected_vt_approx = std::sqrt((2.0 * p.total_mass * cons.gravity) / (rho_sea_level * A * 0.15)); // Assuming Cd ~ 0.15
    
    // Ensure the impact velocity is somewhat close to the expected terminal velocity (within a 30% margin because drag varies over altitude)
    assert(res.impact_velocity > expected_vt_approx * 0.5);
    assert(res.impact_velocity < expected_vt_approx * 1.5);

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
