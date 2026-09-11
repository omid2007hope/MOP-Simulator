// Copyright (c) 2026 Omid Teimory. All Rights Reserved

// packages
#include <fstream>
#include <iostream>
#include <sstream>

// files
#include "nlohmann/json.hpp"
#include "penetration/versionOne/config_loader.hpp"
// ! Parse JSON input specifically for the AI/automation pipeline
using json = nlohmann::json;

SimulationConfig ConfigLoader::loadSimulationConfig(const std::string& filepath) {
	SimulationConfig simConfig;
	std::ifstream ifs(filepath);
	if (!ifs.is_open()) {
		throw std::runtime_error("Failed to open config file: " + filepath);
	}
	json config;
	ifs >> config;

	int simChoice = config.value("/Simulation/choice"_json_pointer, 1);
	simConfig.choice = simChoice;

	if (simChoice == 1) {
		auto p = config.contains("Projectile") ? config["Projectile"] : json::object();
		simConfig.munition.name = p.value("name", "AI Custom Projectile");
		simConfig.munition.length = p.value("length", 6.2);
		simConfig.munition.diameter = p.value("diameter", 0.8);
		simConfig.munition.curvature_noseReduce = p.value("curvature_noseReduce", 1.2);
		simConfig.munition.total_mass = p.value("total_mass", 14000.0);
		simConfig.munition.explosive_mass = p.value("explosive_mass", 2500.0);
		simConfig.munition.explosive_energy_j_per_kg =
			p.value("explosive_energy_j_per_kg", 5e6);
		simConfig.munition.casing_density = p.value("casing_density", 7850.0);
		simConfig.munition.yield_strength = p.value("yield_strength", 1.5e9);
		simConfig.munition.area_moment_inertia = p.value("area_moment_inertia", 0.02);
		simConfig.munition.elastic_modulus = p.value("elastic_modulus", 200e9);
		simConfig.munition.casing_wall_thickness = p.value("casing_wall_thickness", 0.05);
		simConfig.munition.hugoniot_c0 = p.value("hugoniot_c0", 4570.0);
		simConfig.munition.hugoniot_s = p.value("hugoniot_s", 1.49);
		simConfig.munition.explosive_critical_energy =
			p.value("explosive_critical_energy", 3.0e15);
		simConfig.munition.specific_heat = p.value("specific_heat", 460.0);
		simConfig.munition.melting_point = p.value("melting_point", 1800.0);
		simConfig.munition.heat_of_fusion = p.value("heat_of_fusion", 272000.0);

		simConfig.object.name = config.contains("Target") ? config["Target"].value("name", "AI Custom Target") : "AI Custom Target";
		simConfig.object.layers.clear();
		if (config.contains("Target") && config["Target"].contains("layers") && config["Target"]["layers"].is_array() && !config["Target"]["layers"].empty()) {
			for (const auto& t : config["Target"]["layers"]) {
				TargetLayer customLayer;
				customLayer.material_name = t.value("material_name", "AI Custom Layer");
				customLayer.thickness = t.value("thickness", 60.0);
				customLayer.rebar_volume_fraction = t.value("rebar_volume_fraction", 0.02);
				customLayer.rebar_yield_strength = t.value("rebar_yield_strength", 400e6);
				customLayer.density = t.value("density", 2400.0);
				customLayer.compressive_strength = t.value("compressive_strength", 70e6);
				customLayer.hugoniot_c0 = t.value("hugoniot_c0", 3200.0);
				customLayer.hugoniot_s = t.value("hugoniot_s", 1.9);
				customLayer.specific_heat = t.value("specific_heat", 880.0);
				customLayer.melting_point = t.value("melting_point", 1500.0);
				customLayer.heat_of_fusion = t.value("heat_of_fusion", 400000.0);
				simConfig.object.layers.push_back(customLayer);
			}
		} else {
			TargetLayer customLayer;
			customLayer.material_name = "AI Custom Layer (Fallback)";
			customLayer.thickness = 60.0;
			customLayer.rebar_volume_fraction = 0.02;
			customLayer.rebar_yield_strength = 400e6;
			customLayer.density = 2400.0;
			customLayer.compressive_strength = 70e6;
			customLayer.hugoniot_c0 = 3200.0;
			customLayer.hugoniot_s = 1.9;
			customLayer.specific_heat = 880.0;
			customLayer.melting_point = 1500.0;
			customLayer.heat_of_fusion = 400000.0;
			simConfig.object.layers.push_back(customLayer);
		}

		if (config.contains("Aircraft")) {
			auto a = config["Aircraft"];
			simConfig.bomber.name = a.value("name", "AI Custom Bomber");
			simConfig.bomber.bomber_totalMass = a.value("bomber_totalMass", 160000.0);
			simConfig.bomber.bomber_wingArea = a.value("bomber_wingArea", 478.0);
			simConfig.bomber.bomber_liftCurveSlope = a.value("bomber_liftCurveSlope", 4.5);
		} else {
			simConfig.bomber.name = "Unknown Aircraft";
			simConfig.bomber.bomber_totalMass = 0.0;
			simConfig.bomber.bomber_wingArea = 0.0;
			simConfig.bomber.bomber_liftCurveSlope = 0.0;
		}

		if (config.contains("AtmosphereState")) {
			auto env = config["AtmosphereState"];
			simConfig.atmos.temperature_K = env.value("temperature_K", 288.15);
			simConfig.atmos.pressure_Pa = env.value("pressure_Pa", 101325.0);
			simConfig.atmos.density_kgm3 = env.value("density_kgm3", 1.225);
			simConfig.atmos.speed_of_sound_ms = env.value("speed_of_sound_ms", 340.3);
		} else {
			simConfig.atmos.temperature_K = 288.15;
			simConfig.atmos.pressure_Pa = 101325.0;
			simConfig.atmos.density_kgm3 = 1.225;
			simConfig.atmos.speed_of_sound_ms = 340.3;
		}

		if (config.contains("Scenarios") && config["Scenarios"].is_array()) {
			for (const auto& s : config["Scenarios"]) {
				double alt = s.value("altitude_ft", 40000.0);
				double vel = s.value("velocity", 0.0);
				double fpa = s.value("flight_path_angle", 90.0);
				double obliq = s.value("obliquity_angle", 0.0);
				double aoa = s.value("angle_of_attack", 0.0);
				std::string name = s.value("name", "AI Custom Test");
				simConfig.scenarios.push_back({name, alt, vel, fpa, obliq, aoa});
			}
		} else {
			auto s = config.contains("Scenario") ? config["Scenario"] : json::object();
			double alt = s.value("altitude_ft", 40000.0);
			double vel = s.value("velocity", 0.0);
			double fpa = s.value("flight_path_angle", 90.0);
			double obliq = s.value("obliquity_angle", 0.0);
			double aoa = s.value("angle_of_attack", 0.0);

			int numBombs = config.value("/Simulation/numBombs"_json_pointer, 1);
			for (int i = 0; i < numBombs; ++i) {
				std::stringstream name_ss;
				if (numBombs == 1)
					name_ss << s.value("name", "AI Custom Test");
				else if (i == 0)
					name_ss << "Bomb #1 (Shaft Breaker)";
				else
					name_ss << "Bomb #" << (i + 1) << " (Shaft Direct Strike)";
				simConfig.scenarios.push_back({name_ss.str(), alt, vel, fpa, obliq, aoa});
			}
		}
	} else {
		throw std::runtime_error("simChoice > 1 relies on internal hardcoded data which has been removed. Use simChoice = 1 with a full JSON configuration payload.");
	}

	return simConfig;
}
