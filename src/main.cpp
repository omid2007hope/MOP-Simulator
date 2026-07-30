// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include "simulation.hpp"

void clearCinBuffer()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main()
{
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
    std::cout << "Enter choice [1, 2, or 3]: ";

    int choice = 1;
        if (!(std::cin >> choice)) {
            clearCinBuffer();
            choice = 1;
        }

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

    std::vector<ImpactScenario> scenarios;

        if (choice == 2) {
            std::cout << "\n--- INTERACTIVE CUSTOM PARAMETER INPUT ---\n";
            std::cout << "Enter Projectile Name [or word like Custom_Rod]: ";
            std::string pName;
            std::cin >> pName;
            if (!pName.empty())
                mop.name = pName;

            std::cout << "Enter Projectile Length L (meters) [default 6.2]: ";
            double val;
            if (std::cin >> val && val > 0)
                mop.length = val;
            else
                clearCinBuffer();

            std::cout << "Enter Projectile Diameter d (meters) [default 0.8]: ";
            if (std::cin >> val && val > 0)
                mop.diameter = val;
            else
                clearCinBuffer();

            std::cout << "Enter Total Mass m (kg) [default 13600]: ";
            if (std::cin >> val && val > 0)
                mop.total_mass = val;
            else
                clearCinBuffer();

            std::cout << "Enter Explosive Mass (kg) [default 2400, enter 0 for kinetic rod]: ";
            if (std::cin >> val && val >= 0)
                mop.explosive_mass = val;
            else
                clearCinBuffer();

            std::cout << "Enter Casing Density rho_p (kg/m^3) [default 7800 for steel, 19300 for "
                         "tungsten]: ";
            if (std::cin >> val && val > 0)
                mop.casing_density = val;
            else
                clearCinBuffer();

            std::cout << "Enter Casing Yield Strength sigma_y (GPa) [default 2.0, enter 0 for "
                         "kinetic rod]: ";
            if (std::cin >> val && val >= 0)
                mop.yield_strength = val * 1e9;
            else
                clearCinBuffer();

            std::cout << "Enter Target Concrete Density rho_t (kg/m^3) [default 2500]: ";
            if (std::cin >> val && val > 0)
                concrete.density = val;
            else
                clearCinBuffer();

            std::cout << "Enter number of custom impact velocities to test [1 to 5]: ";
            int numScenarios = 1;
                if (!(std::cin >> numScenarios) || numScenarios < 1 || numScenarios > 10) {
                    clearCinBuffer();
                    numScenarios = 1;
                }

                for (int i = 0; i < numScenarios; ++i) {
                    std::cout << "  -> Enter Velocity #" << (i + 1)
                              << " (m/s) [e.g., 3500, 2000, 350]: ";
                    double v = 1000.0;
                        if (std::cin >> v && v > 0) {
                            std::stringstream ss;
                            ss << "Custom Test #" << (i + 1) << " (" << v << " m/s)";
                            scenarios.push_back({ss.str(), 0.0, v});
                        }
                        else {
                            clearCinBuffer();
                        }
                }
                if (scenarios.empty()) {
                    scenarios.push_back({"Custom Default Test", 0.0, 2000.0});
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
        else {
            std::cout << "\n[+] Loading standard GBU-57 MOP drop scenarios...\n";
            scenarios = {{"50, 000ft Drop", 50000.0, 1050.0},
                         {"25,000 ft Drop", 25000.0, 950.0},
                         {"18,000 ft Drop", 18000.0, 880.0},
                         {"Subsonic Operational", 15.0, 340.00}};
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
