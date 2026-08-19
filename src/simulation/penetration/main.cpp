// Copyright (c) 2026 Omid Teimory. All Rights Reserved

// packages
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// files
#include "config_loader.hpp"
#include "default.hpp"
#include "simulation.hpp"
#include "telemetry_exporter.hpp"

// ! Clears input stream errors and flushes invalid buffer characters
void safeCin() {
	if (!std::cin) {
		std::cin.clear();
	}
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// ! Reads and validates numeric input entries interactively from console stream
template <typename T>
T getValidInput(const std::string& prompt, bool allowZero = false) {
	T valueEntry;
	while (true) {
		std::cout << prompt;
		if (std::cin >> valueEntry) {
			if (valueEntry > 0 || (allowZero && valueEntry == 0)) {
				safeCin();
				return valueEntry;
			}
		} else if (std::cin.eof()) {
			std::cerr << "\n[!] EOF encountered. Exiting safely to prevent infinite loop.\n";
			exit(1);
		}

		safeCin();
		std::cout << "Invalid Entry, please try again!\n";
	}
}

// ! Main application entry point for launching impact physics simulations
int main(int argc, char* argv[]) {
	int choice = 2;
	std::string basePath = ".";
	bool jsonMode = false;
	std::string jsonFile = "";

	// Parse command line arguments for JSON Config Mode
	for (int i = 1; i < argc; ++i) {
		if (std::string(argv[i]) == "--json-input" && i + 1 < argc) {
			jsonMode = true;
			jsonFile = argv[i + 1];
		}
	}

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
	if (auto p = ConfigLoader::getProjectileByName(projectilesDb, "GBU-57 Massive Ordnance Penetrator (MOP)")) {
		munition = *p;
	}

	std::vector<ImpactScenario> scenarios;

	// ==========================================
	// JSON CONFIG MODE (Automation Pipeline)
	// ==========================================
	if (jsonMode) {
		std::ifstream ifs(jsonFile);
		if (!ifs.is_open()) {
			std::cerr << "Failed to open config file: " << jsonFile << std::endl;
			return 1;
		}
		json config;
		ifs >> config;

		int simChoice = config.value("/Simulation/choice"_json_pointer, 3);
		choice = simChoice;

		if (simChoice == 1) {
			auto p = config["Projectile"];
			munition.name = p.value("name", "AI Custom Projectile");
			munition.length = p.value("length", 6.2);
			munition.diameter = p.value("diameter", 0.8);
			munition.curvature_noseReduce = p.value("curvature_noseReduce", 1.2);
			munition.total_mass = p.value("total_mass", 14000.0);
			munition.explosive_mass = p.value("explosive_mass", 2500.0);
			munition.explosive_energy_j_per_kg = p.value("explosive_energy_j_per_kg", 5e6);
			munition.casing_density = p.value("casing_density", 7850.0);
			munition.yield_strength = p.value("yield_strength", 1.5e9);
			munition.area_moment_inertia = p.value("area_moment_inertia", 0.02);
			munition.elastic_modulus = p.value("elastic_modulus", 200e9);
			munition.casing_wall_thickness = p.value("casing_wall_thickness", 0.05);
			munition.hugoniot_c0 = p.value("hugoniot_c0", 4570.0);
			munition.hugoniot_s = p.value("hugoniot_s", 1.49);
			munition.explosive_critical_energy = p.value("explosive_critical_energy", 3.0e15);

			auto t = config["Target"]["layers"][0];
			object.layers.clear();
			TargetLayer customLayer;
			customLayer.material_name = "AI Custom Layer";
			customLayer.thickness = t.value("thickness", 60.0);
			customLayer.rebar_volume_fraction = t.value("rebar_volume_fraction", 0.02);
			customLayer.rebar_yield_strength = t.value("rebar_yield_strength", 400e6);
			customLayer.density = t.value("density", 2400.0);
			customLayer.compressive_strength = t.value("compressive_strength", 70e6);
			customLayer.hugoniot_c0 = t.value("hugoniot_c0", 3200.0);
			customLayer.hugoniot_s = t.value("hugoniot_s", 1.9);
			object.layers.push_back(customLayer);

			auto s = config["Scenario"];
			double alt = s.value("altitude_ft", 40000.0);
			double vel = s.value("velocity", 0.0);
			double fpa = s.value("flight_path_angle", 90.0);
			double obliq = s.value("obliquity_angle", 0.0);
			double aoa = s.value("angle_of_attack", 0.0);

			int numBombs = config.value("/Simulation/numBombs"_json_pointer, 1);
			for (int i = 0; i < numBombs; ++i) {
				std::stringstream name_ss;
				if (numBombs == 1) name_ss << s.value("name", "AI Custom Test");
				else if (i == 0) name_ss << "Bomb #1 (Shaft Breaker)";
				else name_ss << "Bomb #" << (i + 1) << " (Shaft Direct Strike)";
				scenarios.push_back({name_ss.str(), alt, vel, fpa, obliq, aoa});
			}
		} else if (simChoice == 3) {
			if (auto p = ConfigLoader::getProjectileByName(projectilesDb, "GBU-57 Massive Ordnance Penetrator (MOP)")) {
				munition = *p;
			} else {
				munition = Midnight_Hammer_projectile;
			}
			object = Midnight_Hammer_Target;
			int numBombs = config.value("/Simulation/numBombs"_json_pointer, 2);
			for (int i = 0; i < numBombs; ++i) {
				std::stringstream name_ss;
				if (i == 0) name_ss << "Bomb #1 (Shaft Breaker)";
				else name_ss << "Bomb #" << (i + 1) << " (Shaft Direct Strike)";
				scenarios.push_back({name_ss.str(), 50000.0, 250.0, 0.0, 0.0, 0.0});
			}
		}
	} 
	// ==========================================
	// INTERACTIVE MODE
	// ==========================================
	else {
		std::cout << "===================================================================================================\n";
		std::cout << "                 C++ IMPACT PHYSICS & PENETRATION SIMULATOR V2.8 (WITH 3D VISUALIZATION)                \n";
		std::cout << "===================================================================================================\n\n";

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
				for (auto& c : tos_agree) {
					c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
				}
				if (tos_agree == "Y" || tos_agree == "YES") {
					safeCin();
					std::cout << "\nTerms accepted. Proceeding to simulator...\n\n";
					break;
				} else if (tos_agree == "N" || tos_agree == "NO") {
					safeCin();
					std::cout << "\nAccess Denied. You must agree to the Terms of Service to use this simulator.\n";
					std::cout << "\nPress Enter to exit...";
					std::cin.get();
					return 1;
				}
			} else if (std::cin.eof()) {
				return 1;
			}
			safeCin();
			std::cout << "Invalid Entry. Please type Y or N.\n";
		}

		std::cout << "Select Simulation Mode:\n";
		std::cout << "  [1] Interactive Custom Input (cin values for mass, velocity, density, etc.)\n";
		std::cout << "  [2] Orbital Kinetic Strike Preset (\"Rods from God\" Tungsten Penetrators)\n";
		std::cout << "  [3] Operation Midnight Hammer (12 days war)\n";
		std::cout << "---------------------------------------------------------------------------------------------------\n";

		while (true) {
			std::cout << "Enter choice [1, 2 or 3]: ";
			if (std::cin >> choice && (choice >= 1 && choice <= 3)) {
				std::cout << "Scenario: " << choice << " Confirmed!";
				safeCin();
				break;
			}
			if (std::cin.eof()) exit(1);
			safeCin();
			std::cout << "Invalid Entry, please try again!\n";
		}

		if (choice == 1) {
			std::cout << "\n--- INTERACTIVE CUSTOM PARAMETER INPUT ---\n";
			std::cout << "Enter Projectile Name [or word like Custom_Rod]: ";
			std::string projectileName;
			getline(std::cin, projectileName);
			if (projectileName.empty()) projectileName = "Undefined Projectile";
			munition.name = projectileName;

			munition.length = getValidInput<double>("Enter Projectile Length L (meters): ", false);
			munition.diameter = getValidInput<double>("Enter Projectile Diameter d (meters): ", false);
			munition.curvature_noseReduce = getValidInput<double>("Enter Projectile Nose Curvature Radius R (meters): ", true);
			munition.total_mass = getValidInput<double>("Enter Total Mass m (kg): ", false);

			while (true) {
				munition.explosive_mass = getValidInput<double>("Enter Explosive Mass (kg): ", true);
				if (munition.explosive_mass <= munition.total_mass) break;
				std::cout << "Error: Explosive mass cannot exceed total mass!\n";
			}

			munition.explosive_energy_j_per_kg = getValidInput<double>("Enter Explosive Energy (j/kg): ", true);
			munition.casing_density = getValidInput<double>("Enter Casing Density rho_p (kg/m^3): ", false);
			double yield = getValidInput<double>("Enter Casing Yield Strength sigma_y (GPa): ", true);
			munition.yield_strength = yield * 1e9;
			munition.area_moment_inertia = getValidInput<double>("Enter Area Moment of Inertia (m^4) [e.g., 0.02]: ", true);
			double modulus = getValidInput<double>("Enter Elastic Modulus E (GPa) [e.g., 200]: ", true);
			if (modulus > 0) munition.elastic_modulus = modulus * 1e9;
			munition.casing_wall_thickness = getValidInput<double>("Enter Casing Wall Thickness (meters) [e.g., 0.05]: ", true);
			munition.hugoniot_c0 = getValidInput<double>("Enter Projectile Hugoniot C0 (m/s) [e.g., 4570]: ", true);
			munition.hugoniot_s = getValidInput<double>("Enter Projectile Hugoniot S [e.g., 1.49]: ", true);
			double ec = getValidInput<double>("Enter Explosive Critical Energy Ec (1e15 Pa^2*s) [e.g., 3.0]: ", true);
			if (ec > 0) munition.explosive_critical_energy = ec * 1e15;

			object.layers.clear();
			TargetLayer customLayer;
			customLayer.material_name = "Custom Layer";
			customLayer.thickness = getValidInput<double>("Enter Target Layer Thickness (meters): ", false);
			customLayer.rebar_volume_fraction = getValidInput<double>("Enter Target Rebar Volume Fraction (0.0 to 1.0, e.g., 0.02): ", true);
			double rebarYield = getValidInput<double>("Enter Target Rebar Yield Strength (MPa, e.g., 400): ", true);
			customLayer.rebar_yield_strength = rebarYield * 1e6;
			customLayer.density = getValidInput<double>("Enter Target Concrete Density rho_t (kg/m^3): ", false);
			double targetStrength = getValidInput<double>("Enter Target Compressive Strength (MPa): ", false);
			customLayer.compressive_strength = targetStrength * 1e6;
			double tc0 = getValidInput<double>("Enter Target Hugoniot C0 (m/s) [e.g., 3200]: ", true);
			if (tc0 > 0) customLayer.hugoniot_c0 = tc0;
			double ts = getValidInput<double>("Enter Target Hugoniot S [e.g., 1.9]: ", true);
			if (ts > 0) customLayer.hugoniot_s = ts;

			object.layers.push_back(customLayer);

			int numScenarios = 1;
			while (true) {
				std::cout << "Enter number of custom impact velocities to test [1 to 5]: ";
				if (std::cin >> numScenarios && numScenarios >= 1 && numScenarios <= 5) {
					safeCin();
					break;
				} else {
					safeCin();
					std::cout << "Invalid Entry, please try again!\n";
				};
				if (std::cin.eof()) {
					std::cerr << "\n[!] EOF encountered. Exiting safely to prevent infinite loop.\n";
					exit(1);
				}
				safeCin();
				std::cout << "Invalid Entry, please try again!\n";
			}

			for (int i = 0; i < numScenarios; ++i) {
				std::stringstream prompt_ss;
				prompt_ss << "  -> Enter Drop Altitude #" << (i + 1) << " (feet) [e.g., 50000, 20000, 15]: ";
				double dropAltitude_ft = getValidInput<double>(prompt_ss.str(), true);

				std::stringstream vel_ss;
				vel_ss << "  -> Enter Initial Velocity #" << (i + 1) << " (m/s) [0 for atmospheric drop from rest]: ";
				double initial_velocity = getValidInput<double>(vel_ss.str(), true);

				std::stringstream obliq_ss;
				obliq_ss << "  -> Enter Obliquity Angle #" << (i + 1) << " (Degrees, 0 for perpendicular): ";
				double obliquity = getValidInput<double>(obliq_ss.str(), true);

				std::stringstream fpa_ss;
				fpa_ss << "  -> Enter Flight Path Angle #" << (i + 1) << " (Degrees, 0=horizontal, 90=down vertical, 270=up vertical): ";
				double fpa = getValidInput<double>(fpa_ss.str(), true);

				std::stringstream aoa_ss;
				aoa_ss << "  -> Enter Angle of Attack #" << (i + 1) << " (Degrees): ";
				double aoa = getValidInput<double>(aoa_ss.str(), true);

				std::stringstream name_ss;
				name_ss << "Custom Test #" << (i + 1) << " (" << dropAltitude_ft << " ft drop)";
				scenarios.push_back({name_ss.str(), dropAltitude_ft, initial_velocity, fpa, obliquity, aoa});
			}
		} else if (choice == 2) {
			std::cout << "\n[+] Loading Orbital Kinetic Strike Preset (\"Rods from God\" Tungsten Penetrators)...\n";
			if (auto p = ConfigLoader::getProjectileByName(projectilesDb, "Orbital Tungsten Kinetic Penetrator (Rods from God)")) {
				munition = *p;
			} else {
				munition = RODS_FROM_GOD_DEFAULT;
			}
			scenarios = {{"Orbital Strike", 3000000.0, 7800.0, 90.0, 0.0, 0.0}};
		} else {
			std::cout << "\n[+] Loading Operation Midnight Hammer Preset (Fordow Subterranean Shaft Strike)\n";
			if (auto p = ConfigLoader::getProjectileByName(projectilesDb, "GBU-57 Massive Ordnance Penetrator (MOP)")) {
				munition = *p;
			} else {
				munition = Midnight_Hammer_projectile;
			}
			object = Midnight_Hammer_Target;

			int numBombs = 2;
			std::cout << "Select Operation Midnight Hammer Salvo Configuration:\n";
			std::cout << "  [1] 2-Bomb Lead Strike (Initial Overburden Shaft Breaker + Follow-on Strike)\n";
			std::cout << "  [2] 4-Bomb Shaft Sequence (Fordow Centrifuge Vault Destruction)\n";
			std::cout << "  [3] 6-Bomb Maximum Salvo Strike (Maximum Subterranean Overpressure Wave)\n";
			int hammerChoice = 1;
			while (true) {
				std::cout << "Selection (1-3): ";
				if (std::cin >> hammerChoice && hammerChoice >= 1 && hammerChoice <= 3) {
					safeCin();
					break;
				}
				if (std::cin.eof()) exit(1);
				safeCin();
				std::cout << "Invalid selection! Enter 1, 2, or 3.\n";
			}

			if (hammerChoice == 1) numBombs = 2;
			else if (hammerChoice == 2) numBombs = 4;
			else numBombs = 6;

			for (int i = 0; i < numBombs; ++i) {
				std::stringstream name_ss;
				if (i == 0) {
					name_ss << "Bomb #1 (Shaft Breaker)";
				} else {
					name_ss << "Bomb #" << (i + 1) << " (Shaft Direct Strike)";
				}
				scenarios.push_back({name_ss.str(), 50000.0, 250.0, 0.0, 0.0, 0.0});
			}
		}
	} // end interactive mode

	// Run simulations
	PhysicsConstants cons;
	std::vector<SimulationResult> results;

	if (choice == 1 || choice == 2) {
		for (const auto& sc : scenarios) {
			ImpactSimulator simulator(munition, object, cons);
			results.push_back(simulator.simulate(sc));
		}
		TelemetryExporter::printReport(results, munition, object);
		TelemetryExporter::generateHtml3DVisualizer(results, munition, object, basePath);
	} else {
		ImpactSimulator simulator(munition, object, cons);
		for (const auto& sc : scenarios) {
			results.push_back(simulator.simulate(sc));
		}
		TelemetryExporter::printReport(results, munition, simulator.getTarget());
		TelemetryExporter::generateHtml3DVisualizer(results, munition, simulator.getTarget(), basePath);
	}

	// Only wait for Enter key if running interactively
	if (!jsonMode) {
		std::cout << "\nPress Enter to exit...";
		std::cin.get();
	}

	return 0;
}
