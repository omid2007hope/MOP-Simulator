// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#include <cassert>
#include <cmath>
#include <iostream>
#include "default.hpp"
#include "simulation.hpp"
#include "config_loader.hpp"

bool approxEqual(double a, double b, double epsilon = 1e-4)
{
    return std::fabs(a - b) < epsilon;
}

int main()
{
    std::cout << "===================================================================================================\n";
    std::cout << "                              RUNNING SIMULATION UNIT TEST SUITE                                   \n";
    std::cout << "===================================================================================================\n";

    // Initialize test target and projectile using defaults
    CONCRETE_DEFAULT concreteDef;
    Target concrete {concreteDef.default_name, concreteDef.default_density, 200.0e6};

    MOP_DEFAULT mopDef;
    Projectile mop {mopDef.default_name, mopDef.default_length, mopDef.default_diameter,
                    mopDef.default_total_mass, mopDef.default_explosive_mass,
                    mopDef.default_casing_density, mopDef.default_yield_strength};

    PhysicsConstants cons;
    ImpactSimulator simulator(mop, concrete, cons);

    // Test 1: Subsonic Operational Impact (Mach ~1.0, 340 m/s)
    std::cout << "[Test 1] Testing Subsonic Rigid Body Penetration (340 m/s)...\n";
    ImpactScenario subScenario {"Subsonic Test", 15.0, 340.0};
    SimulationResult resSub = simulator.simulate(subScenario);

    double expectedKE_sub = 0.5 * mop.total_mass * (340.0 * 340.0);
    double expectedPDyn_sub = 0.5 * concrete.density * (340.0 * 340.0);
    double expectedHydro_depth = mop.length * std::sqrt(mop.casing_density / concrete.density);

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
    ImpactScenario hyperScenario {"Hypervelocity Test", 50000.0, 1500.0};
    SimulationResult resHyper = simulator.simulate(hyperScenario);

    double expectedKE_hyper = 0.5 * mop.total_mass * (1500.0 * 1500.0);
    double expectedPDyn_hyper = 0.5 * concrete.density * (1500.0 * 1500.0);

    assert(approxEqual(resHyper.kinetic_energy, expectedKE_hyper, 1.0));
    assert(approxEqual(resHyper.dynamic_pressure, expectedPDyn_hyper, 1.0));
    assert(resHyper.casing_failure == true);
    assert(resHyper.premature_detonation == true);
    assert(resHyper.regime == "Hydrodynamic / Hypervelocity");
    std::cout << "         -> [PASS] Kinetic Energy: " << (resHyper.kinetic_energy / 1e9) << " GJ\n";
    std::cout << "         -> [PASS] Dynamic Pressure: " << (resHyper.dynamic_pressure / 1e9) << " GPa (> 2.0 GPa Yield)\n";
    std::cout << "         -> [PASS] Hydrodynamic failure and premature detonation correctly detected.\n";
    std::cout << "         -> [PASS] Hydrodynamic Penetration Limit: " << resHyper.hydro_penetration << " m\n\n";

    // Test 3: Orbital Kinetic Strike ("Rods from God" Tungsten Rod, 3400 m/s)
    std::cout << "[Test 3] Testing Orbital Tungsten Kinetic Rod (3400 m/s, 0 GPa Yield, 0 Explosive Mass)...\n";
    
    RODS_FROM_GOD_DEFAULT rfgDef;
    Projectile rod {rfgDef.default_name, rfgDef.default_length, rfgDef.default_diameter,
                    rfgDef.default_total_mass, rfgDef.default_explosive_mass,
                    rfgDef.default_casing_density, rfgDef.default_yield_strength};
                    
    ImpactSimulator rodSim(rod, concrete, cons);
    ImpactScenario rodScenario {"LEO Strike", 100000.0, 3400.0};
    SimulationResult resRod = rodSim.simulate(rodScenario);

    assert(resRod.is_kinetic_rod == true);
    assert(resRod.casing_failure == false);
    assert(resRod.explosive_charge_survives == true);
    assert(resRod.regime == "Hypervelocity Kinetic Rod Penetration");
    std::cout << "         -> [PASS] Identified as Kinetic Rod (Zero explosive mass & zero yield limit).\n";
    std::cout << "         -> [PASS] Regime: " << resRod.regime << "\n";
    std::cout << "         -> [PASS] Depth reached via hydrodynamic plasma erosion: " << resRod.actual_penetration_depth << " m\n\n";

    std::cout << "===================================================================================================\n";
    std::cout << "                      [PASS] ALL UNIT TESTS PASSED SUCCESSFULLY!                                   \n";
    std::cout << "===================================================================================================\n";

    return 0;
}
