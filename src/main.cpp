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

// files
#include "config_loader.hpp"
#include "default.hpp"
#include "simulation.hpp"
#include "telemetry_exporter.hpp"


// ! Clears input stream errors and flushes invalid buffer characters
void safeCin() {
	// comment why if -- reset error state if stream is degraded
	if (!std::cin) {
		std::cin.clear();
	}
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	// **** Ends Here ****
}




// ! Reads and validates numeric input entries interactively from console stream
template <typename T>
T getValidInput(const std::string& prompt, bool allowZero = false) {
	T valueEntry;
	while (true) {
		std::cout << prompt;
		// comment why if -- parse value and validate positivity rules
		if (std::cin >> valueEntry) {
			// comment why if -- check bounds condition
			if (valueEntry > 0 || (allowZero && valueEntry == 0)) {
				safeCin();
				return valueEntry;
			}
		} else if (std::cin.eof()) {
			std::cerr
				<< "\n[!] EOF encountered. Exiting safely to prevent infinite loop.\n";
			exit(1);
		}

		safeCin();
		std::cout << "Invalid Entry, please try again!\n";
	}
	// **** Ends Here ****
}




// ! Main application entry point for launching impact physics simulations
int main(int argc, char* argv[]) {
	int choice = 2;
	std::string basePath = ".";


	// comment why if -- locate base asset path dynamically
	if (argc > 0) {
		std::error_code ec;
		std::filesystem::path current =
			std::filesystem::absolute(argv[0], ec).parent_path();

		while (!current.empty() && current != current.root_path()) {
			// comment why if -- check data directory existence
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

	// **********
	// ! no load from JSON
	// **********

	// Default target
	Target object = CONCRETE_DEFAULT;

	// comment why if -- use target from JSON database if available
	if (auto t = ConfigLoader::getTargetByName(targetsDb,
						   "High-Quality Hardened Concrete Structure")) {
		object = *t;
	}

	// Default projectile (GBU-57 MOP)
	Projectile munition = MOP_DEFAULT;
	// comment why if -- use projectile from JSON database if available
	if (auto p = ConfigLoader::getProjectileByName(
		    projectilesDb, "GBU-57 Massive Ordnance Penetrator (MOP)")) {
		munition = *p;
	}


	std::cout
		<< "================================================================================="
		   "==================\n";
	std::cout << "                 C++ IMPACT PHYSICS & PENETRATION SIMULATOR V2.8 (WITH 3D "
		     "VISUALIZATION)                \n";
	std::cout
		<< "================================================================================="
		   "==================\n\n";

	std::cout << "[!] END-USER LICENSE AGREEMENT (EULA) & TERMS OF SERVICE [!]\n";
	std::cout << "WARNING: This software is a high-fidelity, advanced physics and penetration "
		     "simulator.\n";
	std::cout
		<< "Usage of this application is strictly restricted to recreational, educational, "
		   "and hobbyist purposes.\n";
	std::cout
		<< "Due to the extreme accuracy and sensitive nature of the simulated models, any "
		   "unauthorized, commercial, or malicious application may result in severe legal "
		   "consequences.\n\n";
	std::cout
		<< "DISCLAIMER OF WARRANTY: This software is provided \"AS IS\", without warranty of "
		   "any kind, express or implied.\n";
	std::cout << "LIMITATION OF LIABILITY: In no event shall the author(s) be liable for any "
		     "claim, damages, or other liability\n";
	std::cout << "arising from, out of, or in connection with the software or the use or other "
		     "dealings in the software.\n";
	std::cout
		<< "By proceeding, you acknowledge that this tool is not certified for real-world "
		   "engineering, defense analysis, or physical destructive testing.\n\n";


	std::string tos_agree;
	while (true) {

		std::cout << "Do you agree to these terms? (Y/N): ";

		// comment why if -- validate user agreement to legal terms
		if (std::cin >> tos_agree) {
			for (auto& c : tos_agree) {
				c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
			}
			// comment why if -- handle positive response
			if (tos_agree == "Y" || tos_agree == "YES") {
				safeCin();
				std::cout << "\nTerms accepted. Proceeding to simulator...\n\n";
				break;
			} else if (tos_agree == "N" || tos_agree == "NO") {
				safeCin();
				std::cout
					<< "\nAccess Denied. You must agree to the Terms of Service "
					   "to use this simulator.\n";
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
	std::cout
		<< "  [1] Interactive Custom Input (cin values for mass, velocity, density, etc.)\n";
	std::cout
		<< "  [2] Orbital Kinetic Strike Preset (\"Rods from God\" Tungsten Penetrators)\n";
	std::cout << "  [3] Operation Midnight Hammer (12 days war)\n";
	std::cout
		<< "---------------------------------------------------------------------------------"
		   "------------------\n";

	while (true) {

		std::cout << "Enter choice [1, 2 or 3]: ";

		// comment why if -- check valid scenario selection range
		if (std::cin >> choice && (choice >= 1 && choice <= 3)) {
			std::cout << "Scenario: " << choice << " Confirmed!";
			safeCin();
			break;
		}
		// comment why if -- terminate on end-of-file signal
		if (std::cin.eof())
			exit(1);
		safeCin();
		std::cout << "Invalid Entry, please try again!\n";
	}

	std::vector<ImpactScenario> scenarios;

	// comment why if -- evaluate chosen scenario mode
	if (choice == 1) {
		std::cout << "\n--- INTERACTIVE CUSTOM PARAMETER INPUT ---\n";
		std::cout << "Enter Projectile Name [or word like Custom_Rod]: ";
		std::string projectileName;
		getline(std::cin, projectileName);
		// comment why if -- set default projectile name if omitted
		if (projectileName.empty())
			projectileName = "Undefined Projectile";

		munition.name = projectileName;

		munition.length =
			getValidInput<double>("Enter Projectile Length L (meters): ", false);

		munition.diameter =
			getValidInput<double>("Enter Projectile Diameter d (meters): ", false);

		munition.curvature_noseReduce = getValidInput<double>(
			"Enter Projectile Nose Curvature Radius R (meters): ", true);

		munition.total_mass = getValidInput<double>("Enter Total Mass m (kg): ", false);

		while (true) {
			munition.explosive_mass =
				getValidInput<double>("Enter Explosive Mass (kg): ", true);
			// comment why if -- verify explosive mass constraint
			if (munition.explosive_mass <= munition.total_mass)
				break;
			std::cout << "Error: Explosive mass (" << munition.explosive_mass
				  << " kg) cannot exceed total mass (" << munition.total_mass
				  << " kg)!\n";
		}

		munition.explosive_energy_j_per_kg =
			getValidInput<double>("Enter Explosive Energy (j/kg): ", true);

		munition.casing_density =
			getValidInput<double>("Enter Casing Density rho_p (kg/m^3): ", false);

		double yield =
			getValidInput<double>("Enter Casing Yield Strength sigma_y (GPa): ", true);

		munition.yield_strength = yield * 1e9;

		munition.area_moment_inertia = getValidInput<double>(
			"Enter Area Moment of Inertia (m^4) [e.g., 0.02]: ", true);

		double modulus =
			getValidInput<double>("Enter Elastic Modulus E (GPa) [e.g., 200]: ", true);
		// comment why if -- set non-zero elastic modulus
		if (modulus > 0)
			munition.elastic_modulus = modulus * 1e9;

		munition.casing_wall_thickness = getValidInput<double>(
			"Enter Casing Wall Thickness (meters) [e.g., 0.05]: ", true);

		munition.hugoniot_c0 = getValidInput<double>(
			"Enter Projectile Hugoniot C0 (m/s) [e.g., 4570]: ", true);

		munition.hugoniot_s =
			getValidInput<double>("Enter Projectile Hugoniot S [e.g., 1.49]: ", true);

		double ec = getValidInput<double>(
			"Enter Explosive Critical Energy Ec (1e15 Pa^2*s) [e.g., 3.0]: ", true);
		// comment why if -- set non-zero critical energy
		if (ec > 0)
			munition.explosive_critical_energy = ec * 1e15;

		object.layers.clear();
		TargetLayer customLayer;
		customLayer.material_name = "Custom Layer";
		customLayer.thickness =
			getValidInput<double>("Enter Target Layer Thickness (meters): ", false);

		customLayer.rebar_volume_fraction = getValidInput<double>(
			"Enter Target Rebar Volume Fraction (0.0 to 1.0, e.g., 0.02): ", true);

		double rebarYield = getValidInput<double>(
			"Enter Target Rebar Yield Strength (MPa, e.g., 400): ", true);
		customLayer.rebar_yield_strength = rebarYield * 1e6;

		customLayer.density = getValidInput<double>(
			"Enter Target Concrete Density rho_t (kg/m^3): ", false);

		double targetStrength =
			getValidInput<double>("Enter Target Compressive Strength (MPa): ", false);
		customLayer.compressive_strength = targetStrength * 1e6;

		double tc0 = getValidInput<double>("Enter Target Hugoniot C0 (m/s) [e.g., 3200]: ",
						   true);
		// comment why if -- set target sound speed
		if (tc0 > 0)
			customLayer.hugoniot_c0 = tc0;

		double ts = getValidInput<double>("Enter Target Hugoniot S [e.g., 1.9]: ", true);
		// comment why if -- set target Hugoniot slope
		if (ts > 0)
			customLayer.hugoniot_s = ts;

		object.layers.push_back(customLayer);

		int numScenarios = 1;
		while (true) {

			std::cout << "Enter number of custom impact velocities to test [1 to 5]: ";

			// comment why if -- check valid scenario count
			if (std::cin >> numScenarios && numScenarios >= 1 && numScenarios <= 5) {
				safeCin();
				break;
			} else {
				safeCin();
				std::cout << "Invalid Entry, please try again!\n";
			};

			// comment why if -- exit safely on EOF
			if (std::cin.eof()) {
				std::cerr << "\n[!] EOF encountered. Exiting safely to prevent "
					     "infinite loop.\n";
				exit(1);
			}
			safeCin();
			std::cout << "Invalid Entry, please try again!\n";
		}

		for (int i = 0; i < numScenarios; ++i) {

			std::stringstream prompt_ss;

			prompt_ss << "  -> Enter Drop Altitude #" << (i + 1)
				  << " (feet) [e.g., 50000, 20000, 15]: ";

			double dropAltitude_ft = getValidInput<double>(prompt_ss.str(), true);

			std::stringstream vel_ss;
			vel_ss << "  -> Enter Initial Velocity #" << (i + 1)
			       << " (m/s) [0 for atmospheric drop from rest]: ";
			double initial_velocity = getValidInput<double>(vel_ss.str(), true);

			std::stringstream obliq_ss;
			obliq_ss << "  -> Enter Obliquity Angle #" << (i + 1)
				 << " (Degrees, 0 for perpendicular): ";
			double obliquity = getValidInput<double>(obliq_ss.str(), true);

			std::stringstream fpa_ss;
			fpa_ss << "  -> Enter Flight Path Angle #" << (i + 1)
			       << " (Degrees, 0=horizontal, 90=down vertical, 270=up vertical): ";
			double fpa = getValidInput<double>(fpa_ss.str(), true);

			std::stringstream aoa_ss;
			aoa_ss << "  -> Enter Angle of Attack #" << (i + 1) << " (Degrees): ";
			double aoa = getValidInput<double>(aoa_ss.str(), true);

			std::stringstream name_ss;

			name_ss << "Custom Test #" << (i + 1) << " (" << dropAltitude_ft
				<< " ft drop)";
			scenarios.push_back({name_ss.str(),
					     dropAltitude_ft,
					     initial_velocity,
					     fpa,
					     obliquity,
					     aoa});
		}
	}

	// ! ********************
	// ! Orbital Kinetic Strike Preset
	// ! ********************

	else if (choice == 2) {
		std::cout
			<< "\n[+] Loading Orbital Kinetic Strike Preset (\"Rods from God\" Tungsten "
			   "Penetrators)...\n";

		// comment why if -- load rods from god projectile configuration
		if (auto p = ConfigLoader::getProjectileByName(
			    projectilesDb, "Orbital Tungsten Kinetic Penetrator (Rods from God)")) {
			munition = *p;
		} else {
			munition = RODS_FROM_GOD_DEFAULT;
		}

		scenarios = {
			{"Orbital Strike", 3000000.0, 7800.0, 90.0, 0.0, 0.0},
		};
	}

	// ! ********************
	// ! Midnight Hammer Preset
	// ! ********************

	else {
		std::cout << "\n[+] Loading Operation Midnight Hammer Preset (Fordow Subterranean "
			     "Shaft Strike)\n";

		// comment why if -- load MOP preset projectile configuration
		if (auto p = ConfigLoader::getProjectileByName(
			    projectilesDb, "GBU-57 Massive Ordnance Penetrator (MOP)")) {
			munition = *p;
		} else {
			munition = Midnight_Hammer_projectile;
		}

		object = Midnight_Hammer_Target;

		int numBombs = 2;
		std::cout << "Select Operation Midnight Hammer Salvo Configuration:\n";
		std::cout
			<< "  [1] 2-Bomb Lead Strike (Initial Overburden Shaft Breaker + Follow-on "
			   "Strike)\n";
		std::cout << "  [2] 4-Bomb Shaft Sequence (Fordow Centrifuge Vault Destruction)\n";
		std::cout
			<< "  [3] 6-Bomb Maximum Salvo Strike (Maximum Subterranean Overpressure Wave)\n";
		int hammerChoice = 1;
		while (true) {

			std::cout << "Selection (1-3): ";

			// comment why if -- confirm salvo choice range
			if (std::cin >> hammerChoice && hammerChoice >= 1 && hammerChoice <= 3) {
				safeCin();
				break;
			}
			// comment why if -- check EOF on salvo choice
			if (std::cin.eof())
				exit(1);
			safeCin();
			std::cout << "Invalid selection! Enter 1, 2, or 3.\n";
		}

		// comment why if -- set total bombs in salvo
		if (hammerChoice == 1)
			numBombs = 2;
		else if (hammerChoice == 2)
			numBombs = 4;
		else
			numBombs = 6;

		for (int i = 0; i < numBombs; ++i) {
			std::stringstream name_ss;
			// comment why if -- format bomb name label
			if (i == 0) {
				name_ss << "Bomb #1 (Shaft Breaker)";
			} else {
				name_ss << "Bomb #" << (i + 1) << " (Shaft Direct Strike)";
			}
			scenarios.push_back({name_ss.str(), 50000.0, 250.0, 0.0, 0.0, 0.0});
		}
	}

	// Run simulations
	PhysicsConstants cons;

	std::vector<SimulationResult> results;

	// comment why if -- evaluate single vs sequential simulation modes
	if (choice == 1 || choice == 2) {
		// Independent tests get fresh targets
		// ! scenarios -> scenarios data e.g. inital velocity, altitude, etc
		for (const auto& sc : scenarios) {
			// ! cons -> physics constant e.g. gravity on earth
			// ! object -> target data e.g. depth
			// ! munition -> projectiles data e.g. explosive mass
			ImpactSimulator simulator(munition, object, cons);


			results.push_back(simulator.simulate(sc));
		}

		TelemetryExporter::printReport(results, munition, object);
		TelemetryExporter::generateHtml3DVisualizer(results, munition, object, basePath);
	} else {
		// Sequential multi-bomb strikes use a persistent simulator instance to maintain
		// cumulative shaft crater depth
		ImpactSimulator simulator(munition, object, cons);
		for (const auto& sc : scenarios) {
			results.push_back(simulator.simulate(sc));
		}
		TelemetryExporter::printReport(results, munition, simulator.getTarget());
		TelemetryExporter::generateHtml3DVisualizer(
			results, munition, simulator.getTarget(), basePath);
	}

	std::cout << "\nPress Enter to exit...";
	std::cin.get();

	return 0;
	// **** Ends Here ****
}
