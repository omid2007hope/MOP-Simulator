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
    Target object = CONCRETE_DEFAULT;
        if (auto t = ConfigLoader::getTargetByName(targetsDb, "High-Quality Hardened Concrete")) {
            object = *t;
        }

    // Default projectile (GBU-57 MOP)
    Projectile munition = MOP_DEFAULT;
        if (auto p = ConfigLoader::getProjectileByName(
                projectilesDb, "GBU-57 Massive Ordnance Penetrator (MOP)")) {
            munition = *p;
        }

    std::cout << "================================================================================="
                 "==================\n";
    std::cout << "                 C++ IMPACT PHYSICS & PENETRATION SIMULATOR V2.8 (WITH 3D "
                 "VISUALIZATION)                \n";
    std::cout << "================================================================================="
                 "==================\n\n";
                 
    std::cout << "[!] END-USER LICENSE AGREEMENT (EULA) & TERMS OF SERVICE [!]\n";
    std::cout << "WARNING: This software is a high-fidelity, advanced physics and penetration simulator.\n";
    std::cout << "Usage of this application is strictly restricted to recreational, educational, and hobbyist purposes.\n";
    std::cout << "Due to the extreme accuracy and sensitive nature of the simulated models, any unauthorized, commercial, or malicious application may result in severe legal consequences.\n\n";
    std::cout << "DISCLAIMER OF WARRANTY: This software is provided \"AS IS\", without warranty of any kind, express or implied.\n";
    std::cout << "LIMITATION OF LIABILITY: In no event shall the author(s) be liable for any claim, damages, or other liability\n";
    std::cout << "arising from, out of, or in connection with the software or the use or other dealings in the software.\n";
    std::cout << "By proceeding, you acknowledge that this tool is not certified for real-world engineering, defense analysis, or physical destructive testing.\n\n";
    
    std::string tos_agree;
    while (true) {
        std::cout << "Do you agree to these terms? (Y/N): ";
        if (std::cin >> tos_agree) {
            if (tos_agree == "Y" || tos_agree == "y" || tos_agree == "YES" || tos_agree == "yes" || tos_agree == "Yes") {
                safeCin();
                std::cout << "\nTerms accepted. Proceeding to simulator...\n\n";
                break;
            } else if (tos_agree == "N" || tos_agree == "n" || tos_agree == "NO" || tos_agree == "no" || tos_agree == "No") {
                std::cout << "\nAccess Denied. You must agree to the Terms of Service to use this simulator.\n";
                system("pause");
                return 1;
            }
        }
        if (std::cin.eof()) return 1;
        safeCin();
        std::cout << "Invalid Entry. Please type Y or N.\n";
    }

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
            customLayer.thickness = getValidInput<double>("Enter Target Layer Thickness (meters): ", false);
            
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

                for (int i = 0; i < numScenarios; ++i) {
                    std::stringstream prompt_ss;

                    prompt_ss << "  -> Enter Drop Altitude #" << (i + 1)
                              << " (feet) [e.g., 50000, 20000, 15]: ";

                    double dropAltitude_ft = getValidInput<double>(prompt_ss.str());
                    
                    std::stringstream obliq_ss;
                    obliq_ss << "  -> Enter Obliquity Angle #" << (i + 1) << " (Degrees, 0 for perpendicular): ";
                    double obliquity = getValidInput<double>(obliq_ss.str(), true);

                    std::stringstream aoa_ss;
                    aoa_ss << "  -> Enter Angle of Attack #" << (i + 1) << " (Degrees): ";
                    double aoa = getValidInput<double>(aoa_ss.str(), true);

                    std::stringstream name_ss;

                    name_ss << "Custom Test #" << (i + 1) << " (" << dropAltitude_ft
                            << " ft drop)";
                    scenarios.push_back({name_ss.str(), dropAltitude_ft, 0.0, obliquity, aoa});
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
                    munition = RODS_FROM_GOD_DEFAULT;
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

    std::cout << "\n";
    system("pause");

    return 0;
}
