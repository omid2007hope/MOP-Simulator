#include "simulation.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

bool approxEqual(double a, double b, double epsilon = 1e-4) {
    return std::fabs(a - b) < epsilon;
}

int main() {
    std::cout << "===================================================================================================\n";
    std::cout << "                              RUNNING SIMULATION UNIT TEST SUITE                                   \n";
    std::cout << "===================================================================================================\n";

    // Initialize test target and projectile
    Target concrete{"Hardened Concrete", 2500.0};
    Projectile mop{"GBU-57 MOP", 6.2, 0.8, 13600.0, 2400.0, 7800.0, 2.0e9};

    ImpactSimulator simulator(mop, concrete);

    // Test 1: Subsonic Operational Impact (Mach ~1.0, 340 m/s)
    std::cout << "[Test 1] Testing Subsonic Rigid Body Penetration (340 m/s)...\n";
    ImpactScenario subScenario{"Subsonic Test", 15.0, 340.0};
    SimulationResult resSub = simulator.simulate(subScenario);

    double expectedKE_sub = 0.5 * 13600.0 * (340.0 * 340.0);
    double expectedPDyn_sub = 0.5 * 2500.0 * (340.0 * 340.0);
    double expectedHydro_depth = 6.2 * std::sqrt(7800.0 / 2500.0);

    assert(approxEqual(resSub.kinetic_energy, expectedKE_sub, 1.0));
    assert(approxEqual(resSub.dynamic_pressure, expectedPDyn_sub, 1.0));
    assert(approxEqual(resSub.hydro_penetration, expectedHydro_depth, 1e-3));
    assert(resSub.casing_failure == false);
    assert(resSub.premature_detonation == false);
    assert(resSub.regime == "Rigid Body Penetration");
    std::cout << "         -> [PASS] Kinetic Energy: " << (resSub.kinetic_energy / 1e9) << " GJ\n";
    std::cout << "         -> [PASS] Dynamic Pressure: " << (resSub.dynamic_pressure / 1e9) << " GPa (< 2.0 GPa Yield)\n";
    std::cout << "         -> [PASS] Casing remained intact in rigid penetration regime.\n\n";

    // Test 2: Hypervelocity Impact (Mach ~14.7, 1500 m/s)
    std::cout << "[Test 2] Testing Hypervelocity Hydrodynamic Failure (1500 m/s)...\n";
    ImpactScenario hyperScenario{"Hypervelocity Test", 50000.0, 1500.0};
    SimulationResult resHyper = simulator.simulate(hyperScenario);

    double expectedKE_hyper = 0.5 * 13600.0 * (1500.0 * 1500.0);
    double expectedPDyn_hyper = 0.5 * 2500.0 * (1500.0 * 1500.0);

    assert(approxEqual(resHyper.kinetic_energy, expectedKE_hyper, 1.0));
    assert(approxEqual(resHyper.dynamic_pressure, expectedPDyn_hyper, 1.0));
    assert(resHyper.casing_failure == true);
    assert(resHyper.premature_detonation == true);
    assert(resHyper.regime == "Hydrodynamic / Hypervelocity");
    std::cout << "         -> [PASS] Kinetic Energy: " << (resHyper.kinetic_energy / 1e9) << " GJ\n";
    std::cout << "         -> [PASS] Dynamic Pressure: " << (resHyper.dynamic_pressure / 1e9) << " GPa (> 2.0 GPa Yield)\n";
    std::cout << "         -> [PASS] Hydrodynamic failure and premature detonation correctly detected.\n";
    std::cout << "         -> [PASS] Hydrodynamic Penetration Limit: " << resHyper.hydro_penetration << " m\n\n";

    std::cout << "===================================================================================================\n";
    std::cout << "                      [PASS] ALL UNIT TESTS PASSED SUCCESSFULLY!                                   \n";
    std::cout << "===================================================================================================\n";

    return 0;
}
