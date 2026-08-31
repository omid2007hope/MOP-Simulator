// Copyright (c) 2026 Omid Teimory. All Rights Reserved

// packages
#include <fstream>
#include <iostream>

// files
#include "config_loader.hpp"
#include "nlohmann/json.hpp"


using json = nlohmann::json;


// ! Loads target specifications from JSON configuration file
std::vector<Target> ConfigLoader::loadTargets(const std::string& filepath) {
	std::vector<Target> targets;
	std::ifstream file(filepath);
	// comment why if -- warn user and return fallback target list if config file is missing
	if (!file.is_open()) {
		std::cerr << "Warning: Could not open " << filepath << ". Using defaults.\n";
		return targets;
	}

	try {
		json j;
		file >> j;
		for (const auto& item : j) {
			Target t;
			t.name = item.value("name", "Unknown Target");
			// comment why if -- parse multi-layer array if present, otherwise build legacy single layer
			if (item.contains("layers") && item["layers"].is_array()) {
				for (const auto& l : item["layers"]) {
					TargetLayer layer;
					layer.material_name =
						l.value("material_name", "Unknown Material");
					layer.thickness = l.value("thickness", 1.0);
					layer.density = l.value("density", 2500.0);
					layer.compressive_strength =
						l.value("compressive_strength", 60.0e6);
					layer.rebar_volume_fraction =
						l.value("rebar_volume_fraction", 0.0);
					layer.rebar_yield_strength =
						l.value("rebar_yield_strength", 0.0);
					layer.hugoniot_c0 = l.value("hugoniot_c0", 3200.0);
					layer.hugoniot_s = l.value("hugoniot_s", 1.9);
					t.layers.push_back(layer);
				}
			} else {
				TargetLayer layer;
				layer.material_name = "Legacy Material";
				layer.thickness = 100.0; // Assume semi-infinite
				layer.density = item.value("density", 2500.0);
				layer.compressive_strength =
					item.value("compressive_strength", 60.0e6);
				layer.rebar_volume_fraction = 0.0;
				layer.rebar_yield_strength = 0.0;
				layer.hugoniot_c0 = item.value("hugoniot_c0", 3200.0);
				layer.hugoniot_s = item.value("hugoniot_s", 1.9);
				t.layers.push_back(layer);
			}
			targets.push_back(t);
		}
	} catch (const std::exception& e) {
		std::cerr << "Error parsing JSON in " << filepath << ": " << e.what() << "\n";
	}

	return targets;
	// **** Ends Here ****
}