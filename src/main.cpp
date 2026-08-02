// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#include <filesystem>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include "config_loader.hpp"
#include "default.hpp"
#include "simulation.hpp"

void safeCin()
{
        if (!std::cin) {
            std::cin.clear();
        }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Helper to safely read numeric input and re-prompt on failure
template <typename T>
T getValidInput(const std::string& prompt, bool allowZero = false)
{
    T valueEntry;
        while (true) {
            std::cout << prompt;
                if (std::cin >> valueEntry &&
                    (valueEntry > 0 || (allowZero && valueEntry == 0))) {
                    return valueEntry;
                }
                if (std::cin.eof()) {
                    std::cerr
                        << "\n[!] EOF encountered. Exiting safely to prevent infinite loop.\n";
                    exit(1);
                }
            safeCin();
            std::cout << "Invalid Entry, please try again!\n";
        }
}

int main(int argc, char* argv[])
{
    int choice = 2;

    std::string basePath = ".";
        if (argc > 0) {
            std::error_code ec;
            std::filesystem::path current = std::filesystem::absolute(argv[0], ec).parent_path();
            while (!current.empty() && current != current.root_path()) {
                if (std::filesystem::exists(current / "data" / "targets.json", ec)) {
                    basePath = current.string();
                    break;
                }
                current = current.parent_path();
            }
        }

    // Load databases
    auto targetsDb = ConfigLoader::loadTargets(basePath + "/data/targets.json");
    auto projectilesDb = ConfigLoader::loadProjectiles(basePath + "/data/projectiles.json");

    // Default target
    Target object;
        if (auto t = ConfigLoader::getTargetByName(targetsDb, "High-Quality Hardened Concrete")) {
            object = *t;
        }
        else {
            CONCRETE_DEFAULT objectDefaultValue;
            object.name = objectDefaultValue.default_name;
            object.layers = objectDefaultValue.default_layers;
        }

    // Default projectile (GBU-57 MOP)
    Projectile munition;
        if (auto p = ConfigLoader::getProjectileByName(
                projectilesDb, "GBU-57 Massive Ordnance Penetrator (MOP)")) {
            munition = *p;
        }
        else {
            MOP_DEFAULT mopDefaultValue;

            munition.name = mopDefaultValue.default_name;
            munition.length = mopDefaultValue.default_length;
            munition.diameter = mopDefaultValue.default_diameter;
            munition.curvature_noseReduce = mopDefaultValue.curvature_noseReduce;
            munition.total_mass = mopDefaultValue.default_total_mass;
            munition.explosive_mass = mopDefaultValue.default_explosive_mass;
            munition.casing_density = mopDefaultValue.default_casing_density;
            munition.yield_strength = mopDefaultValue.default_yield_strength;
            munition.specific_heat = mopDefaultValue.default_specific_heat;
            munition.melting_point = mopDefaultValue.default_melting_point;
            munition.heat_of_fusion = mopDefaultValue.default_heat_of_fusion;
            munition.area_moment_inertia = mopDefaultValue.default_area_moment_inertia;
        }

    std::cout << "================================================================================="
                 "==================\n";
    std::cout << "                 C++ IMPACT PHYSICS & PENETRATION SIMULATOR V2.5 (WITH 3D "
                 "VISUALIZATION)                \n";
    std::cout << "================================================================================="
                 "==================\n";
    std::cout << "Select Simulation Mode:\n";
    std::cout << "  [1] Run Standard GBU-57 MOP Presets (Mach 1.0 to Mach 10.4)\n";
    std::cout << "  [2] Interactive Custom Input (cin values for mass, velocity, density, etc.)\n";
    std::cout << "  [3] Orbital Kinetic Strike Preset (\"Rods from God\" Tungsten Penetrators)\n";
    std::cout << "---------------------------------------------------------------------------------"
                 "------------------\n";

        while (true) {
            std::cout << "Enter choice [1, 2, or 3]: ";
                if (std::cin >> choice && (choice == 1 || choice == 2 || choice == 3)) {
                    std::cout << "Scenario: " << choice << " Confirmed!";
                    safeCin();
                    break;
                }
            if (std::cin.eof())
                exit(1);
            safeCin();
            std::cout << "Invalid Entry, please try again!\n";
        }

    std::vector<ImpactScenario> scenarios;

        if (choice == 2) {
            std::cout << "\n--- INTERACTIVE CUSTOM PARAMETER INPUT ---\n";
            std::cout << "Enter Projectile Name [or word like Custom_Rod]: ";
            std::string projectileName;
            getline(std::cin, projectileName);
            if (projectileName.empty())
                projectileName = "Undefined Projectile";

            munition.name = projectileName;

            munition.length = getValidInput<double>("Enter Projectile Length L (meters): ", false);

            munition.diameter =
                getValidInput<double>("Enter Projectile Diameter d (meters): ", false);

            munition.curvature_noseReduce =
                getValidInput<double>("Enter Projectile Nose Curvature Radius R (meters): ", true);

            munition.total_mass = getValidInput<double>("Enter Total Mass m (kg): ", false);

                while (true) {
                    munition.explosive_mass =
                        getValidInput<double>("Enter Explosive Mass (kg): ", true);
                    if (munition.explosive_mass <= munition.total_mass)
                        break;
                    std::cout << "Error: Explosive mass (" << munition.explosive_mass
                              << " kg) cannot exceed total mass (" << munition.total_mass
                              << " kg)!\n";
                }

            munition.casing_density =
                getValidInput<double>("Enter Casing Density rho_p (kg/m^3): ", false);

            double yield =
                getValidInput<double>("Enter Casing Yield Strength sigma_y (GPa): ", true);

            munition.yield_strength = yield * 1e9;

            munition.area_moment_inertia =
                getValidInput<double>("Enter Area Moment of Inertia (m^4) [e.g., 0.02]: ", true);

            object.layers.clear();
            TargetLayer customLayer;
            customLayer.material_name = "Custom Layer";
            customLayer.thickness = 100.0;
            
            customLayer.rebar_volume_fraction =
                getValidInput<double>("Enter Target Rebar Volume Fraction (0.0 to 1.0, e.g., 0.02): ", true);
            
            double rebarYield = 
                getValidInput<double>("Enter Target Rebar Yield Strength (MPa, e.g., 400): ", true);
            customLayer.rebar_yield_strength = rebarYield * 1e6;
            
            customLayer.density =
                getValidInput<double>("Enter Target Concrete Density rho_t (kg/m^3): ", false);

            double targetStrength = getValidInput<double>("Enter Target Compressive Strength (MPa): ", false);
            customLayer.compressiveStrength = targetStrength * 1e6;
            object.layers.push_back(customLayer);

            int numScenarios = 1;
                while (true) {
                    std::cout << "Enter number of custom impact velocities to test [1 to 5]: ";
                        if (std::cin >> numScenarios && numScenarios >= 1 && numScenarios <= 5) {
                            break;
                        }
                    if (std::cin.eof()) {
                        std::cerr << "\n[!] EOF encountered. Exiting safely to prevent infinite loop.\n";
                        exit(1);
                    }
                    safeCin();
                    std::cout << "Invalid Entry, please try again!\n";
                }

                for (int each = 0; each < numScenarios; ++each) {
                    std::stringstream prompt_ss;

                    prompt_ss << "  -> Enter Velocity #" << (each + 1)
                              << " (m/s) [e.g., 3500, 2000, 350]: ";

                    double projectileVelocity = getValidInput<double>(prompt_ss.str());
                    
                    std::stringstream obliq_ss;
                    obliq_ss << "  -> Enter Obliquity Angle #" << (each + 1) << " (Degrees, 0 for perpendicular): ";
                    double obliquity = getValidInput<double>(obliq_ss.str(), true);

                    std::stringstream aoa_ss;
                    aoa_ss << "  -> Enter Angle of Attack #" << (each + 1) << " (Degrees): ";
                    double aoa = getValidInput<double>(aoa_ss.str(), true);

                    std::stringstream name_ss;

                    name_ss << "Custom Test #" << (each + 1) << " (" << projectileVelocity
                            << " m/s)";
                    scenarios.push_back({name_ss.str(), 0.0, projectileVelocity, obliquity, aoa});
                }

                if (scenarios.empty()) {
                    scenarios.push_back({"Custom Default Test", 50000.0, 500.0, 0.0, 0.0});
                }
        }
        else if (choice == 3) {
            std::cout << "\n[+] Loading Orbital Kinetic Strike Preset (\"Rods from God\" Tungsten "
                         "Penetrators)...\n";

                if (auto p = ConfigLoader::getProjectileByName(
                        projectilesDb, "Orbital Tungsten Kinetic Penetrator (Rods from God)")) {
                    munition = *p;
                }
                else {
                    RODS_FROM_GOD_DEFAULT rodsFromGodDefaultValue;

                    munition.name = rodsFromGodDefaultValue.default_name;
                    munition.length = rodsFromGodDefaultValue.default_length;
                    munition.diameter = rodsFromGodDefaultValue.default_diameter;
                    munition.curvature_noseReduce = rodsFromGodDefaultValue.curvature_noseReduce;
                    munition.total_mass = rodsFromGodDefaultValue.default_total_mass;
                    munition.explosive_mass =
                        rodsFromGodDefaultValue
                            .default_explosive_mass; // 0 kg explosive (pure kinetic energy weapon)
                    munition.casing_density =
                        rodsFromGodDefaultValue.default_casing_density; // High-density Tungsten
                    munition.yield_strength =
                        rodsFromGodDefaultValue.default_yield_strength; 
                    munition.specific_heat = rodsFromGodDefaultValue.default_specific_heat;
                    munition.melting_point = rodsFromGodDefaultValue.default_melting_point;
                    munition.heat_of_fusion = rodsFromGodDefaultValue.default_heat_of_fusion;
                    munition.area_moment_inertia = rodsFromGodDefaultValue.default_area_moment_inertia;
                }

            scenarios = {{"LEO Orbital Strike (Mach 10)", 100000.0, 3400.0, 0.0, 0.0},
                         {"Deep Orbital Strike (Mach 15)", 200000.0, 5100.0, 0.0, 0.0},
                         {"Hypervelocity Terminal (Mach 22)", 300000.0, 7500.0, 0.0, 0.0}};
        }

        else { // choice == 1 or fallback
            std::cout << "\n[+] Loading standard GBU-57 MOP drop scenarios...\n";

            scenarios = {
                {"50,000 ft Drop", 50000.0, 500.0, 0.0, 0.0},
                {"45,000 ft Drop", 45000.0, 400.0, 0.0, 0.0},
                {"Subsonic Operational", 40000.0, 300.0, 0.0, 0.0},
            };
        }

    // Initialize Simulator
    PhysicsConstants cons;
    ImpactSimulator simulator(munition, object, cons);

    // Run simulations
    std::vector<SimulationResult> results;
        for (const auto& sc : scenarios) {
            results.push_back(simulator.simulate(sc));
        }

    // Print report and ASCII 3D cross-sections
    simulator.printReport(results);

    // Generate 3D HTML WebGL visualizer
    simulator.generateHtml3DVisualizer(results, basePath);

    return 0;
}
