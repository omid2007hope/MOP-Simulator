// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#include "telemetry_exporter.hpp"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace TelemetryExporter {

void printAscii3DVisualizer(const SimulationResult& r,
			    const Projectile& proj,
			    const Target& target) {
	std::cout
		<< "\n==============================================================================="
		   "====================\n";
	std::cout << "                 ASCII 3D CROSS-SECTION VISUALIZATION: [" << r.scenario_name
		  << "]\n";
	std::cout
		<< "================================================================================="
		   "==================\n";
	std::cout << "  Velocity: " << std::fixed << std::setprecision(1) << r.velocity
		  << " m/s (Mach " << r.mach_number << ") | Dyn. Press: " << std::setprecision(2)
		  << (r.dynamic_pressure / 1e9)
		  << " GPa | Casing Yield: " << (proj.yield_strength / 1e9) << " GPa\n";
	std::cout << "  Actual Penetration Depth: " << r.actual_penetration_depth << " m ("
		  << r.actual_penetration_depth * 3.28084 << " ft) | "
		  << "Shock Failure Chance: " << std::setprecision(1) << r.shock_damage_prob_percent
		  << "% | "
		  << "Explosive Survived: "
		  << (r.explosive_charge_survives ? "YES" : "NO (SHOCK/CRUSH)") << "\n";
	std::cout
		<< "---------------------------------------------------------------------------------"
		   "------------------\n\n";

	if (r.is_kinetic_rod) {
		std::cout << "        AIR / SPACE             |  KINETIC STRIKE ROD APPROACH (Mach "
			  << std::setprecision(1) << r.mach_number << ")\n";
		std::cout
			<< "                                |       ||=======||    (Solid Tungsten / "
			   "Heavy Metal)\n";
		std::cout
			<< "                                |       ||=======||    (Zero Explosive "
			   "Mass / 0 Yield Mode)\n";
		std::cout << "                                |       ||=======||    \n";
		std::cout
			<< "    "
			   "============================+=======||=======||=========================="
			   "== [Target Surface]\n";
		std::cout
			<< "     *    *   *  *  *  *  *  *  |       ||       ||   << HYPERVELOCITY "
			   "KINETIC EROSION >>       |\n";
		std::cout << "    *   [ KINETIC CRATERING ] * |       ||       ||   P_dyn = "
			  << std::setprecision(2) << (r.dynamic_pressure / 1e9)
			  << " GPa                  |\n";
		std::cout
			<< "     *    *   *  *  *  *  *  *  |       ||=======||   (Penetrating via "
			   "hydrodynamic ratio)      |\n";
		std::cout << "    "
			     ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~\\.~.~.~./"
			     ".~.~.~.~.~.~.~.~.~.~.~.~.~.~. [Deep Kinetic Channel]\n";
		double target_density = target.layers.empty() ? 2500.0 : target.layers[0].density;
		std::cout
			<< "    .   Concrete Target         |         \\     /     Max Penetration: "
			<< std::setprecision(1) << r.actual_penetration_depth << " meters    |\n";
		std::cout << "    .   (Density: " << target_density
			  << " kg/m^3) |          \\___/      ("
			  << r.actual_penetration_depth * 3.28084 << " feet deep into target) |\n";
		std::cout
			<< "    .                           |              *                         "
			   "       |\n";
		std::cout
			<< "    "
			   ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~"
			   ".~ [Bottom of Kinetic Crater]\n";
	} else if (r.casing_failure || !r.explosive_charge_survives) {
		std::cout << "        AIR / SURFACE           |  PROJECTILE APPROACH (Mach "
			  << std::setprecision(1) << r.mach_number << ")\n";
		std::cout
			<< "                                |       ||=======||    (Hardened Steel "
			   "Casing)\n";
		std::cout
			<< "                                |       ||   *   ||    (Sensitive High "
			   "Explosive)\n";
		std::cout << "                                |       ||=======||    \n";
		std::cout
			<< "    "
			   "============================+=======++=======++=========================="
			   "== [Target Surface]\n";
		std::cout
			<< "     *    *   *  *  *  *  *  *  |   << IMPACT SHOCK / FAILURE ZONE >>    "
			   "     |\n";
		std::cout << "    *  ["
			  << (r.casing_failure ? "SURFACE DETONATION!" : "SHOCK FUZE FAILURE!")
			  << "] * |   (P_dyn = " << std::setprecision(1)
			  << (r.dynamic_pressure / 1e9)
			  << " GPa, Shock Damage = " << std::setprecision(0)
			  << r.shock_damage_prob_percent << "%)     |\n";
		std::cout
			<< "     *    *   *  *  *  *  *  *  |                                        "
			   "     |\n";
		std::cout
			<< "    "
			   ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~"
			   ".~ [Erosion / Crater Zone]\n";
		std::cout
			<< "    .   Concrete Target         |   Casing/Payload damaged upon impact.  "
			   "     |\n";
		double target_density2 = target.layers.empty() ? 2500.0 : target.layers[0].density;
		std::cout << "    .   (Density: " << target_density2
			  << " kg/m^3) |   Max Penetration Depth:                    |\n";
		std::cout << "    .                           |   D = " << std::setprecision(2)
			  << r.actual_penetration_depth << " m ("
			  << r.actual_penetration_depth * 3.28084
			  << " ft)                         |\n";
		std::cout
			<< "    "
			   ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~"
			   ".~ [Depth Limit: "
			<< r.actual_penetration_depth << "m]\n";
		std::cout << "    .                           |   (Payload fails to reach deep "
			     "tunnels)     |\n";
	} else {
		std::cout
			<< "        AIR / SURFACE           |  PROJECTILE APPROACH (Subsonic Mach "
			<< std::setprecision(1) << r.mach_number << ")\n";
		std::cout
			<< "                                |       ||=======||    (Hardened Steel "
			   "Casing)\n";
		std::cout
			<< "                                |       ||   *   ||    (Sensitive High "
			   "Explosive)\n";
		std::cout << "                                |       ||=======||    \n";
		std::cout
			<< "    "
			   "============================+=======||=======||=========================="
			   "== [Target Surface]\n";
		std::cout
			<< "                                |       ||       ||                      "
			   "     |\n";
		std::cout << "     [ RIGID PENETRATION ]      |       ||       ||   P_dyn = "
			  << std::setprecision(2) << (r.dynamic_pressure / 1e9)
			  << " GPa           |\n";
		std::cout << "     (Casing intact & payload   |       ||   *   ||   (Shock Damage: "
			  << std::setprecision(1) << r.shock_damage_prob_percent << "%)      |\n";
		std::cout
			<< "      survives impact shock)    |       ||=======||                      "
			   "         |\n";
		std::cout << "                                |       ||   v   ||   Rigid Depth: "
			  << std::setprecision(1) << r.rigid_penetration << " m          |\n";
		std::cout << "    "
			     ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~\\.~.~.~./"
			     ".~.~.~.~.~.~.~.~.~.~.~.~.~.~. [Drilling Deep into Rock]\n";
		std::cout
			<< "    .   Concrete Target         |         \\     /                       "
			   "      |\n";
		double target_density3 = target.layers.empty() ? 2500.0 : target.layers[0].density;
		std::cout << "    .   (Density: " << target_density3
			  << " kg/m^3) |          \\___/  <-- Reaches " << std::setprecision(1)
			  << r.actual_penetration_depth << " meters    |\n";
		std::cout << "    .                           |              *      ("
			  << r.actual_penetration_depth * 3.28084 << " feet underground) |\n";
		std::cout
			<< "    "
			   ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~"
			   ".~ [Deep Underground Fuze Trigger]\n";
	}
	std::cout
		<< "================================================================================="
		   "==================\n\n";
}

void printReport(const std::vector<SimulationResult>& results,
		 const Projectile& proj,
		 const Target& target) {
	std::cout
		<< "\n==============================================================================="
		   "====================\n";
	std::cout
		<< "                      PROJECTILE IMPACT & PENETRATION SIMULATION REPORT          "
		   "                  \n";
	std::cout
		<< "================================================================================="
		   "==================\n";
	std::cout << "Projectile : " << proj.name << "\n";
	std::cout << "  - Length         : " << proj.length << " m (" << proj.length * 3.28084
		  << " ft)\n";
	std::cout << "  - Total Mass     : " << proj.total_mass << " kg\n";
	std::cout << "  - Explosive Mass : " << proj.explosive_mass << " kg ("
		  << (proj.explosive_mass == 0 ? "KINETIC ROD / 0 EXPLOSIVE"
					       : "CONVENTIONAL CHARGE")
		  << ")\n";
	std::cout << "  - Casing Density : " << proj.casing_density << " kg/m^3\n";
	std::cout << "  - Yield Strength : " << proj.yield_strength / 1e9 << " GPa\n";
	std::cout << "Target     : " << target.name << " (Layers: " << target.layers.size()
		  << ")\n";
	std::cout
		<< "---------------------------------------------------------------------------------"
		   "------------------\n";

	double total_thickness = 0.0;
	double weighted_density_sum = 0.0;
	for (const auto& layer : target.layers) {
		weighted_density_sum += layer.density * layer.thickness;
		total_thickness += layer.thickness;
	}
	double default_density = target.layers.empty() ? 2500.0 : target.layers[0].density;
	double average_density =
		(total_thickness > 0) ? (weighted_density_sum / total_thickness) : default_density;

	std::cout
		<< "Alekseevskii-Tate Hydrodynamic Limit (Avg Target Density): P = L * sqrt(rho_p / "
		   "rho_t) = "
		<< std::fixed << std::setprecision(2)
		<< proj.length * std::sqrt(proj.casing_density / average_density) << " m ("
		<< proj.length * std::sqrt(proj.casing_density / average_density) * 3.28084
		<< " ft)\n";
	std::cout
		<< "================================================================================="
		   "==================\n\n";

	// Print Summary Table
	std::cout << std::left << std::setw(20) << "Scenario" << std::right << std::setw(12)
		  << "Velocity" << std::right << std::setw(9) << "Mach" << std::right
		  << std::setw(14) << "Dyn.Press" << std::right << std::setw(12) << "Depth (m)"
		  << std::right << std::setw(12) << "Shock Dmg"
		  << "  " << std::left << std::setw(30) << "Regime" << std::left << std::setw(20)
		  << "Outcome" << "\n";
	std::cout << std::string(130, '-') << "\n";

	for (const auto& r : results) {
		std::cout << std::left << std::setw(20) << r.scenario_name << std::right
			  << std::setw(8) << std::fixed << std::setprecision(1) << r.velocity
			  << " m/s" << std::right << std::setw(6) << std::fixed
			  << std::setprecision(1) << r.mach_number << "x" << std::right
			  << std::setw(9) << std::fixed << std::setprecision(2)
			  << (r.dynamic_pressure / 1e9) << " GPa" << std::right << std::setw(9)
			  << std::fixed << std::setprecision(1) << r.actual_penetration_depth
			  << " m" << std::right << std::setw(10) << std::fixed
			  << std::setprecision(0) << r.shock_damage_prob_percent << "%"
			  << "  " << std::left << std::setw(30) << r.regime << std::left
			  << std::setw(20) << r.outcome_summary << "\n";
	}
	std::cout << std::string(130, '-') << "\n\n";

	// Print ASCII 3D cross sections for all simulated scenarios
	for (const auto& r : results) {
		printAscii3DVisualizer(r, proj, target);
	}
}

void generateHtml3DVisualizer(const std::vector<SimulationResult>& results,
			      const Projectile& proj,
			      const Target& target,
			      const std::string& basePath) {
	std::ifstream tpl(basePath + "/assets/visualizer_template.html");
	if (!tpl.is_open()) {
		std::cerr
			<< "[!] Warning: HTML Visualizer template not found at " << basePath
			<< "/assets/visualizer_template.html. Visualizer will not be generated.\n";
		return;
	}

	std::string filename = basePath + "/3d_visualizer.html";
	std::ofstream out(filename);
	if (!out.is_open()) {
		std::cerr << "Error: Could not open " << filename << " for writing.\n";
		return;
	}

	std::stringstream buffer;
	buffer << tpl.rdbuf();
	std::string html = buffer.str();

	auto escapeJSON = [](const std::string& s) {
		std::string res;
		for (char c : s) {
			if (c == '"')
				res += "\\\"";
			else if (c == '\\')
				res += "\\\\";
			else if (c == '\b')
				res += "\\b";
			else if (c == '\f')
				res += "\\f";
			else if (c == '\n')
				res += "\\n";
			else if (c == '\r')
				res += "\\r";
			else if (c == '\t')
				res += "\\t";
			else
				res += c;
		}
		return res;
	};

	// Generate scenario buttons
	std::stringstream buttons;
	for (size_t i = 0; i < results.size(); ++i) {
		buttons << "        <button onclick=\"selectScenario(" << i
			<< ")\" id=\"btn-scenario-" << i
			<< "\" class=\"scenario-btn px-4 py-1.5 rounded-full text-xs font-semibold "
			   "whitespace-nowrap "
			   "transition-all duration-200 bg-slate-800/80 hover:bg-cyan-500 "
			   "hover:text-slate-950 border border-slate-700/60\">"
			<< escapeJSON(results[i].scenario_name) << "</button>\n";
	}

	// Generate scenario data
	std::stringstream data;
	for (size_t i = 0; i < results.size(); ++i) {
		const auto& r = results[i];
		std::stringstream dropFramesJson;
		dropFramesJson << "[";
		for (size_t j = 0; j < r.drop_frames.size(); ++j) {
			const auto& f = r.drop_frames[j];
			dropFramesJson << "{t:" << f.time << ",y:" << f.altitude
				       << ",v:" << f.velocity << ",m:" << f.mach
				       << ",sb:" << (f.is_sonic_boom ? "true" : "false")
				       << ",pr:" << f.pitch_rad << "}";
			if (j + 1 < r.drop_frames.size())
				dropFramesJson << ",";
		}
		dropFramesJson << "]";

		std::stringstream penFramesJson;
		penFramesJson << "[";
		for (size_t j = 0; j < r.penetration_frames.size(); ++j) {
			const auto& f = r.penetration_frames[j];
			penFramesJson << "{t:" << f.time << ",y:" << f.depth << ",v:" << f.velocity
				      << ",m:" << f.mach << ",p:" << (f.dynamic_pressure / 1.0e9)
				      << ",g:" << f.g_force << ",h:" << f.heat
				      << ",e:" << (f.is_eroding ? "true" : "false")
				      << ",dif:" << f.dif << ",rl:" << f.remaining_length
				      << ",ob:" << f.obliquity_deg << "}";
			if (j + 1 < r.penetration_frames.size())
				penFramesJson << ",";
		}
		penFramesJson << "]";

		std::stringstream targetLayersJson;
		targetLayersJson << "[";
		for (size_t k = 0; k < target.layers.size(); ++k) {
			const auto& lay = target.layers[k];
			targetLayersJson << "{name:\"" << escapeJSON(lay.material_name)
					 << "\",thickness:" << lay.thickness
					 << ",density:" << lay.density << "}";
			if (k + 1 < target.layers.size())
				targetLayersJson << ",";
		}
		targetLayersJson << "]";

		data << "            { name: \"" << escapeJSON(r.scenario_name)
		     << "\", velocity: " << r.velocity << ", mach: " << r.mach_number
		     << ", energy: " << (r.kinetic_energy / 1e9)
		     << ", pressure: " << (r.dynamic_pressure / 1e9)
		     << ", yield: " << (proj.yield_strength / 1e9)
		     << ", depth: " << r.actual_penetration_depth
		     << ", prev_strike_depth: " << r.previous_strike_depth
		     << ", rigid_depth: " << r.rigid_penetration
		     << ", hydro_depth: " << r.hydro_penetration
		     << ", fail: " << (r.casing_failure ? "true" : "false")
		     << ", shock_prob: " << r.shock_damage_prob_percent
		     << ", exp_survives: " << (r.explosive_charge_survives ? "true" : "false")
		     << ", is_kinetic: " << (r.is_kinetic_rod ? "true" : "false") << ", regime: \""
		     << escapeJSON(r.regime) << "\", summary: \"" << escapeJSON(r.outcome_summary)
		     << "\""
		     << ", proj_length: " << proj.length << ", proj_diameter: " << proj.diameter
		     << ", proj_name: \"" << escapeJSON(proj.name) << "\", target_name: \""
		     << escapeJSON(target.name) << "\""
		     << ", explosive_mass: " << r.explosive_mass
		     << ", explosion_scale: " << r.explosion_scale
		     << ", crater_wide_radius: " << r.crater_wide_radius
		     << ", crater_narrow_radius: " << r.crater_narrow_radius
		     << ", camera_shake_magnitude: " << r.camera_shake_magnitude
		     << ", time_scale_pen: " << r.time_scale_pen
		     << ", erosion_occurred: " << (r.erosion_occurred ? "true" : "false")
		     << ", final_rod_length: " << r.final_rod_length
		     << ", erosion_length_lost: " << r.erosion_length_lost
		     << ", dynamic_increase_factor: " << r.dynamic_increase_factor
		     << ", bar_wave_speed: " << r.bar_wave_speed
		     << ", shock_pressure_gpa_peak: " << r.shock_pressure_gpa_peak
		     << ", shock_pulse_duration_us: " << r.shock_pulse_duration_us
		     << ", previous_strike_depth: " << r.previous_strike_depth
		     << ", cumulative_breach_depth: " << r.cumulative_breach_depth
		     << ", target_layers: " << targetLayersJson.str()
		     << ", drop_frames: " << dropFramesJson.str()
		     << ", pen_frames: " << penFramesJson.str() << " }";
		if (i + 1 < results.size())
			data << ",";
		data << "\n";
	}

	auto replaceAll = [](std::string& str, const std::string& from, const std::string& to) {
		if (from.empty())
			return;
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
	};

	replaceAll(html, "{{SCENARIO_BUTTONS}}", buttons.str());
	replaceAll(html, "/*{{SCENARIOS_DATA}}*/", data.str());

	out << html;
	out.close();

	std::cout << "\n[+] Successfully generated 3D WebGL Interactive Visualizer: " << filename
		  << "\n";
	std::cout << "    -> Open " << filename
		  << " in your web browser to view the 3D simulation scene!\n";
}

} // namespace TelemetryExporter
