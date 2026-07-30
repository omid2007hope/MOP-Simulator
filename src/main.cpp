// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include "simulation.hpp"

const bool allowEntry = true;

int choice = 2;
double valueEntry;

// Helper to safely read numeric input and re-prompt on failure
template <typename T>
T getValidInput(const std::string& prompt, T defaultValue, bool allowZero = false)
{
    T valueEntry;
        while (allowEntry == true) {
            std::cout << prompt;
                if (std::cin >> valueEntry &&
                    (valueEntry > 0 || (allowZero == true && valueEntry == 0))) {
                    return valueEntry;
                }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid Entry, please try again!\n";
        }
}

int main()
{
    // Default target
    Target concrete;
    concrete.name = "High-Quality Hardened Concrete";
    concrete.density = 2500.0; // kg/m^3

    // Default projectile (GBU-57 MOP)
    Projectile mop;
    mop.name = "GBU-57 Massive Ordnance Penetrator (MOP)";
    mop.length = 6.2;
    mop.diameter = 0.8;
    mop.total_mass = 13600.0;
    mop.explosive_mass = 2400.0;
    mop.casing_density = 7800.0;
    mop.yield_strength = 2.0e9; // 2.0 GPa

    std::cout << "================================================================================="
                 "==================\n";
    std::cout << "                 C++ IMPACT PHYSICS & PENETRATION SIMULATOR (WITH 3D "
                 "VISUALIZATION)                \n";
    std::cout << "================================================================================="
                 "==================\n";
    std::cout << "Select Simulation Mode:\n";
    std::cout << "  [1] Run Standard GBU-57 MOP Presets (Mach 1.0 to Mach 10.4)\n";
    std::cout << "  [2] Interactive Custom Input (cin values for mass, velocity, density, etc.)\n";
    std::cout << "  [3] Orbital Kinetic Strike Preset (\"Rods from God\" Tungsten Penetrators)\n";
    std::cout << "---------------------------------------------------------------------------------"
                 "------------------\n";

        while (allowEntry == true) {
            std::cout << "Enter choice [1, 2, or 3]: ";
                if (std::cin >> choice && (choice == 1 || choice == 2 || choice == 3)) {
                    std::cout << "Scenario: " << choice << " Confirmed!";
                    break;
                }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid Entry, please try again!\n";
        }

    std::vector<ImpactScenario> scenarios;

        if (choice == 2) {
            std::cout << "\n--- INTERACTIVE CUSTOM PARAMETER INPUT ---\n";
            std::cout << "Enter Projectile Name [or word like Custom_Rod]: ";
            std::string projectileName;
            getline(std::cin, projectileName) if (!projectileName.empty())
                getline(projectileName, "Undefined Projectile");

            mop.length = getValidInput<double>("Enter Projectile Length L (meters): ");

            mop.diameter = getValidInput<double>("Enter Projectile Diameter d (meters): ");

            mop.total_mass = getValidInput<double>("Enter Total Mass m (kg): ");

            mop.explosive_mass = getValidInput<double>("Enter Explosive Mass (kg): ");

            mop.casing_density = getValidInput<double>("Enter Casing Density rho_p (kg/m^3): ");

            double yield = getValidInput<double>("Enter Casing Yield Strength sigma_y (GPa): ");

            mop.yield_strength = yield * 1e9;

            concrete.density =
                getValidInput<double>("Enter Target Concrete Density rho_t (kg/m^3): ");

            int numScenarios = 1;
                while (allowEntry == true) {
                    std::cout << "Enter number of custom impact velocities to test [1 to 5]: ";
                        if (std::cin >> numScenarios && numScenarios >= 1 && numScenarios <= 5) {
                            break;
                        }
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid Entry, please try again!\n";
                }

                for (numScenarios : eachScenario) {
                    std::stringstream prompt_ss;

                    prompt_ss << "  -> Enter Velocity #" << (eachScenario + 1)
                              << " (m/s) [e.g., 3500, 2000, 350]: ";

                    double projectileVelocity = getValidInput<double>(prompt_ss.str());
                    std::stringstream name_ss;

                    name_ss << "Custom Test #" << (eachScenario + 1) << " (" << projectileVelocity
                            << " m/s)";
                    scenarios.push_back({name_ss.str(), 0.0, projectileVelocity});
                }

                if (scenarios.empty()) {
                    scenarios.push_back({"Custom Default Test", 50000.0, 500.0});
                }
        }
        else if (choice == 3) {
            std::cout << "\n[+] Loading Orbital Kinetic Strike Preset (\"Rods from God\" Tungsten "
                         "Penetrators)...\n";

            mop.name = "Orbital Tungsten Kinetic Penetrator (Rods from God)";
            mop.length = 6.1;
            mop.diameter = 0.3;
            mop.total_mass = 8300.0;
            mop.explosive_mass = 0.0;     // 0 kg explosive (pure kinetic energy weapon)
            mop.casing_density = 19300.0; // High-density Tungsten
            mop.yield_strength = 0.0;     // 0 GPa (hydrodynamic erosion dominated at hypervelocity)

            scenarios = {{"LEO Orbital Strike (Mach 10)", 100000.0, 3400.0},
                         {"Deep Orbital Strike (Mach 15)", 200000.0, 5100.0},
                         {"Hypervelocity Terminal (Mach 22)", 300000.0, 7500.0}};
        }

        else { // choice == 1 or fallback
            std::cout << "\n[+] Loading standard GBU-57 MOP drop scenarios...\n";

            scenarios = {
                {"50, 000ft Drop", 50000.0, 500.0},
                {"45,000 ft Drop", 45000.0, 400.0},
                {"Subsonic Operational", 40000.0, 300.0},
            };
        }

    // Initialize Simulator
    ImpactSimulator simulator(mop, concrete);

    // Run simulations
    std::vector<SimulationResult> results;
        for (const auto& sc : scenarios) {
            results.push_back(simulator.simulate(sc));
        }

    // Print report and ASCII 3D cross-sections
    simulator.printReport(results);

    // Generate 3D HTML WebGL visualizer
    simulator.generateHtml3DVisualizer(results);

    return 0;
}
