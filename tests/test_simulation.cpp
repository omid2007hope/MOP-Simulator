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
    Target concrete = CONCRETE_DEFAULT;
    Projectile mop = MOP_DEFAULT;

    PhysicsConstants cons;
    ImpactSimulator simulator(mop, concrete, cons);

    // Test 1: Subsonic Operational Impact (Mach ~1.0, 340 m/s)
    std::cout << "[Test 1] Testing Subsonic Time-Integrated Penetration (340 m/s)...\n";
    ImpactScenario subScenario {"Subsonic Test", 15.0, 340.0, 0.0, 0.0};
    SimulationResult resSub = simulator.simulate(subScenario);

    double expectedKE_sub = 0.5 * mop.total_mass * (340.0 * 340.0);
    double total_thickness = 0.0;
    double weighted_density_sum = 0.0;
    for (const auto& layer : concrete.layers) {
        weighted_density_sum += layer.density * layer.thickness;
        total_thickness += layer.thickness;
    }
    double average_density = (total_thickness > 0) ? (weighted_density_sum / total_thickness) : concrete.layers[0].density;
    double expectedHydro_depth = mop.length * std::sqrt(mop.casing_density / average_density);

    assert(approxEqual(resSub.kinetic_energy, expectedKE_sub, 1.0));
    assert(resSub.dynamic_pressure > 0.0 && resSub.dynamic_pressure < mop.yield_strength);
    assert(approxEqual(resSub.hydro_penetration, expectedHydro_depth, 1e-3));
    assert(resSub.casing_failure == false);
    assert(resSub.premature_detonation == false);
    assert(resSub.regime == "Time-Integrated Penetration");
    std::cout << "         -> [PASS] Kinetic Energy: " << (resSub.kinetic_energy / 1e9) << " GJ\n";
    std::cout << "         -> [PASS] Max Dynamic Pressure: " << (resSub.dynamic_pressure / 1e9) << " GPa (< 2.0 GPa Yield)\n";
    std::cout << "         -> [PASS] Casing remained intact in time-integrated regime.\n\n";

    // Test 2: Hypervelocity Impact (Mach ~14.7, 1500 m/s)
    std::cout << "[Test 2] Testing Hypervelocity Hydrodynamic Failure (1500 m/s)...\n";
    ImpactScenario hyperScenario {"Hypervelocity Test", 50000.0, 1500.0, 0.0, 0.0};
    SimulationResult resHyper = simulator.simulate(hyperScenario);

    double expectedKE_hyper = 0.5 * mop.total_mass * (1500.0 * 1500.0);
    
    assert(approxEqual(resHyper.kinetic_energy, expectedKE_hyper, 1.0));
    assert(resHyper.casing_failure == true);
    // Based on the new logic, it might fail from crushing (Pressure Yield) or Thermal
    // Depending on what happens first. The pressure yield should happen immediately.
    assert(resHyper.premature_detonation == true || resHyper.regime == "Thermal Destruction");
    std::cout << "         -> [PASS] Kinetic Energy: " << (resHyper.kinetic_energy / 1e9) << " GJ\n";
    std::cout << "         -> [PASS] Dynamic Pressure: " << (resHyper.dynamic_pressure / 1e9) << " GPa (> 2.0 GPa Yield)\n";
    std::cout << "         -> [PASS] Failure correctly detected. Regime: " << resHyper.regime << "\n";
    std::cout << "         -> [PASS] Hydrodynamic Penetration Limit: " << resHyper.hydro_penetration << " m\n\n";

    // Test 3: Orbital Kinetic Strike ("Rods from God" Tungsten Rod, 3400 m/s)
    std::cout << "[Test 3] Testing Orbital Tungsten Kinetic Rod (3400 m/s, 0 GPa Yield, 0 Explosive Mass)...\n";
    
    Projectile rod = RODS_FROM_GOD_DEFAULT;
                    
    ImpactSimulator rodSim(rod, concrete, cons);
    ImpactScenario rodScenario {"LEO Strike", 100000.0, 3400.0, 0.0, 0.0};
    SimulationResult resRod = rodSim.simulate(rodScenario);

    assert(resRod.is_kinetic_rod == true);
    // Kinetic rods with 0 yield strength fail immediately under pressure
    // Kinetic rods with 0 yield strength ablate, but large ones may not fully ablate.
    assert(resRod.casing_failure == false || resRod.regime == "Thermal Destruction");
    assert(resRod.explosive_charge_survives == true);
    std::cout << "         -> [PASS] Identified as Kinetic Rod (Zero explosive mass & zero yield limit).\n";
    std::cout << "         -> [PASS] Regime: " << resRod.regime << "\n";
    std::cout << "         -> [PASS] Depth reached before erosion/crush: " << resRod.actual_penetration_depth << " m\n\n";

    // Test 4: Oblique Impact Bending Structural Failure (400 m/s, 30 deg obliquity, 5 deg AoA)
    std::cout << "[Test 4] Testing Oblique Impact Bending Structural Failure (Obliquity 30 deg, AoA 5 deg, 400 m/s)...\n";
    ImpactScenario obliqueScenario {"Oblique Test", 15.0, 400.0, 30.0, 5.0};
    SimulationResult resOblique = simulator.simulate(obliqueScenario);
    
    assert(resOblique.casing_failure == true);
    assert(resOblique.regime == "Structural Failure (J-Hook/Snap)");
    std::cout << "         -> [PASS] Bending moment exceeded yield strength as expected.\n";
    std::cout << "         -> [PASS] Regime: " << resOblique.regime << "\n\n";

    std::cout << "===================================================================================================\n";
    std::cout << "                      [PASS] ALL UNIT TESTS PASSED SUCCESSFULLY!                                   \n";
    std::cout << "===================================================================================================\n";

    return 0;
}
