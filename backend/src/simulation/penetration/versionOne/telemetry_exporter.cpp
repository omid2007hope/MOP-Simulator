// Copyright (c) 2026 Omid Teimory. All Rights Reserved

// packages
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

// files
#include "penetration/versionOne/telemetry_exporter.hpp"


namespace TelemetryExporter {


// ! ============================================================
// ! Single source of truth — all fields in one place.
// ! printReport() and generateHtml3DVisualizer() both call this.
// ! ============================================================
std::string serializeResultToJson(const SimulationResult& r,
				   const Projectile& proj,
				   const Target& target) {

	// --- Helper: serialize a TelemetryFrame vector to a JSON array string ---
	auto serializeDropFrames = [](const std::vector<TelemetryFrame>& frames) -> std::string {
		std::ostringstream ss;
		ss << "[";
		for (size_t i = 0; i < frames.size(); ++i) {
			const auto& f = frames[i];
			ss << "{\"t\":" << f.time
			   << ",\"y\":" << f.altitude
			   << ",\"v\":" << f.velocity
			   << ",\"m\":" << f.mach
			   << ",\"p\":" << f.dynamic_pressure
			   << ",\"sb\":" << (f.is_sonic_boom ? "true" : "false")
			   << ",\"h\":" << f.heat
			   << ",\"g\":" << f.g_force
			   << ",\"pr\":" << f.pitch_rad
			   << ",\"e\":" << (f.is_eroding ? "true" : "false")
			   << ",\"dif\":" << f.dif
			   << ",\"rl\":" << f.remaining_length
			   << ",\"ob\":" << f.obliquity_deg
			   << ",\"cvx\":" << f.current_vx
			   << ",\"cvy\":" << f.current_vy
			   << ",\"dc\":" << f.drag_coefficient
			   << ",\"df\":" << f.drag_force
			   << ",\"gp\":" << f.guidance_pull
			   << "}";
			if (i + 1 < frames.size()) ss << ",";
		}
		ss << "]";
		return ss.str();
	};

	auto serializePenFrames = [](const std::vector<TelemetryFrame>& frames) -> std::string {
		std::ostringstream ss;
		ss << "[";
		for (size_t i = 0; i < frames.size(); ++i) {
			const auto& f = frames[i];
			ss << "{\"t\":" << f.time
			   << ",\"y\":" << f.depth
			   << ",\"v\":" << f.velocity
			   << ",\"m\":" << f.mach
			   << ",\"p\":" << f.dynamic_pressure
			   << ",\"g\":" << f.g_force
			   << ",\"h\":" << f.heat
			   << ",\"e\":" << (f.is_eroding ? "true" : "false")
			   << ",\"dif\":" << f.dif
			   << ",\"rl\":" << f.remaining_length
			   << ",\"ob\":" << f.obliquity_deg
			   << ",\"cvx\":" << f.current_vx
			   << ",\"cvy\":" << f.current_vy
			   << ",\"up\":" << f.Up
			   << ",\"us\":" << f.Us
			   << ",\"ps\":" << f.P_shock
			   << ",\"tp\":" << f.transmitted_pressure
			   << ",\"se\":" << f.shock_energy
			   << ",\"af\":" << f.asymmetric_force
			   << ",\"bm\":" << f.bending_moment
			   << ",\"mbs\":" << f.max_bending_stress
			   << ",\"sr\":" << f.strain_rate
			   << ",\"es\":" << f.effective_strength
			   << ",\"tf\":" << f.tunnel_force
			   << ",\"iev\":" << f.interface_erosion_velocity
			   << ",\"hr\":" << f.heat_rate
			   << ",\"eh\":" << f.excess_heat
			   << ",\"ml\":" << f.mass_loss
			   << ",\"eld\":" << f.effective_linear_density
			   << ",\"sb\":" << (f.is_sonic_boom ? "true" : "false")
			   << ",\"pr\":" << f.pitch_rad
			   << ",\"dc\":" << f.drag_coefficient
			   << ",\"df\":" << f.drag_force
			   << ",\"gp\":" << f.guidance_pull
			   << "}";
			if (i + 1 < frames.size()) ss << ",";
		}
		ss << "]";
		return ss.str();
	};

	// --- Helper: serialize target layers to a JSON array string ---
	auto serializeTargetLayers = [](const Target& tgt) -> std::string {
		std::ostringstream ss;
		ss << "[";
		for (size_t k = 0; k < tgt.layers.size(); ++k) {
			const auto& lay = tgt.layers[k];
			ss << "{\"name\":\"" << lay.material_name << "\""
			   << ",\"thickness\":" << lay.thickness
			   << ",\"density\":" << lay.density
			   << ",\"compressive_strength\":" << lay.compressive_strength
			   << ",\"rebar_volume_fraction\":" << lay.rebar_volume_fraction
			   << ",\"rebar_yield_strength\":" << lay.rebar_yield_strength
			   << ",\"pulverized_depth\":" << lay.pulverized_depth
			   << ",\"hugoniot_c0\":" << lay.hugoniot_c0
			   << ",\"hugoniot_s\":" << lay.hugoniot_s
			   << "}";
			if (k + 1 < tgt.layers.size()) ss << ",";
		}
		ss << "]";
		return ss.str();
	};

	// --- Helper: serialize layer_bottom_depths to a JSON array string ---
	auto serializeLayerDepths = [](const std::vector<double>& depths) -> std::string {
		std::ostringstream ss;
		ss << "[";
		for (size_t k = 0; k < depths.size(); ++k) {
			ss << depths[k];
			if (k + 1 < depths.size()) ss << ",";
		}
		ss << "]";
		return ss.str();
	};

	PhysicsConstants cons{};
	std::ostringstream j;
	j << "{"
	  // Core result
	  << "\"name\":\"" << r.scenario_name << "\""
	  << ",\"velocity\":" << r.velocity
	  << ",\"mach\":" << r.mach_number
	  << ",\"energy\":" << (r.kinetic_energy / 1e9)
	  << ",\"exp_survives\":" << (r.explosive_charge_survives ? "true" : "false")
	  << ",\"is_kinetic\":" << (r.is_kinetic_rod ? "true" : "false")
	  << ",\"regime\":\"" << r.regime << "\""
	  << ",\"summary\":\"" << r.outcome_summary << "\""
	  // Projectile
	  << ",\"proj_name\":\"" << proj.name << "\""
	  << ",\"proj_length\":" << proj.length
	  << ",\"proj_diameter\":" << proj.diameter
	  << ",\"proj_total_mass\":" << proj.total_mass
	  << ",\"proj_curvature_noseReduce\":" << proj.curvature_noseReduce
	  << ",\"proj_casing_density\":" << proj.casing_density
	  << ",\"proj_casing_wall_thickness\":" << proj.casing_wall_thickness
	  << ",\"proj_area_moment_inertia\":" << proj.area_moment_inertia
	  << ",\"proj_elastic_modulus\":" << proj.elastic_modulus
	  << ",\"proj_hugoniot_c0\":" << proj.hugoniot_c0
	  << ",\"proj_hugoniot_s\":" << proj.hugoniot_s
	  << ",\"proj_explosive_energy_j_per_kg\":" << proj.explosive_energy_j_per_kg
	  << ",\"proj_specific_heat\":" << proj.specific_heat
	  << ",\"proj_melting_point\":" << proj.melting_point
	  << ",\"proj_heat_of_fusion\":" << proj.heat_of_fusion
	  // Target
	  << ",\"target_name\":\"" << target.name << "\""
	  << ",\"flight_path_angle\":" << r.flight_path_angle
	  << ",\"obliquity_angle\":" << r.obliquity_angle
	  << ",\"angle_of_attack\":" << r.angle_of_attack
	  // Physics constants
	  << ",\"cons_gravity\":" << cons.gravity
	  << ",\"cons_pi\":" << cons.PI
	  << ",\"cons_friction_factor\":" << cons.frictionFactor
	  << ",\"cons_speed_of_sound\":" << cons.SpeedOfSound
	  << ",\"cons_universalGasConstant\":" << r.cons_universalGasConstant
	  << ",\"cons_molarMassAir\":" << r.cons_molarMassAir
	  << ",\"cons_adiabaticIndexAir\":" << r.cons_adiabaticIndexAir
	  << ",\"cons_earthRadius\":" << r.cons_earthRadius
	  // Explosion & crater
	  << ",\"explosive_mass\":" << r.explosive_mass
	  << ",\"explosion_scale\":" << r.explosion_scale
	  << ",\"crater_wide_radius\":" << r.crater_wide_radius
	  << ",\"crater_narrow_radius\":" << r.crater_narrow_radius
	  << ",\"camera_shake_magnitude\":" << r.camera_shake_magnitude
	  << ",\"time_scale_pen\":" << r.time_scale_pen
	  << ",\"yield\":" << r.total_explosive_yield
	  << ",\"fail\":" << (r.premature_detonation ? "true" : "false")
	  // Erosion
	  << ",\"erosion_occurred\":" << (r.erosion_occurred ? "true" : "false")
	  << ",\"final_rod_length\":" << r.final_rod_length
	  << ",\"erosion_length_lost\":" << r.erosion_length_lost
	  // Shock & dynamics
	  << ",\"dynamic_increase_factor\":" << r.dynamic_increase_factor
	  << ",\"bar_wave_speed\":" << r.bar_wave_speed
	  << ",\"shock_pressure_gpa_peak\":" << r.shock_pressure_gpa_peak
	  << ",\"shock_pulse_duration_us\":" << r.shock_pulse_duration_us
	  << ",\"previous_strike_depth\":" << r.previous_strike_depth
	  << ",\"cumulative_breach_depth\":" << r.cumulative_breach_depth
	  << ",\"kinetic_shock_joules\":" << r.kinetic_shock_joules
	  // Flight kinematics
	  << ",\"altitude_ft\":" << r.altitude_ft
	  << ",\"x_acceleration\":" << r.x_acceleration
	  << ",\"y_acceleration\":" << r.y_acceleration
	  << ",\"trim_deg\":" << r.trim_deg
	  << ",\"trim_rad\":" << r.trim_rad
	  << ",\"fpa_rad_corrected\":" << r.fpa_rad_corrected
	  << ",\"area\":" << r.area
	  << ",\"boom_time\":" << r.boom_time
	  << ",\"boom_alt\":" << r.boom_alt
	  << ",\"impact_velocity\":" << r.impact_velocity
	  << ",\"impact_pitch\":" << r.impact_pitch
	  << ",\"initial_shaft_depth\":" << r.initial_shaft_depth
	  << ",\"critical_angle_threshold\":" << r.critical_angle_threshold
	  << ",\"average_density\":" << r.average_density
	  // Aircraft platform
	  << ",\"aircraft_bomber_totalMass\":" << r.aircraft_bomber_totalMass
	  << ",\"aircraft_bomber_wingArea\":" << r.aircraft_bomber_wingArea
	  << ",\"aircraft_bomber_liftCurveSlope\":" << r.aircraft_bomber_liftCurveSlope
	  // Arrays
	  << ",\"target_layers\":" << serializeTargetLayers(target)
	  << ",\"layer_bottom_depths\":" << serializeLayerDepths(r.layer_bottom_depths)
	  << ",\"drop_frames\":" << serializeDropFrames(r.drop_frames)
	  << ",\"pen_frames\":" << serializePenFrames(r.penetration_frames)
	  << "}";

	return j.str();
}


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

	auto leftPad = [](const std::string& str, int width) {
		std::stringstream ss;
		ss << std::left << std::setw(width) << str;
		return ss.str();
	};

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
		
		std::stringstream ss_pdyn; 
		ss_pdyn << "P_dyn = " << std::fixed << std::setprecision(2) << (r.dynamic_pressure / 1e9) << " GPa";
		std::cout << "    *   [ KINETIC CRATERING ] * |       ||       ||   "
			  << leftPad(ss_pdyn.str(), 35) << "|\n";
			  
		std::cout
			<< "     *    *   *  *  *  *  *  *  |       ||=======||   (Penetrating via "
			   "hydrodynamic ratio)      |\n";
		std::cout << "    "
			     ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~\\.~.~.~./"
			     ".~.~.~.~.~.~.~.~.~.~.~.~.~.~. [Deep Kinetic Channel]\n";
			     
		double target_density = target.layers.empty() ? 2500.0 : target.layers[0].density;
		
		std::stringstream ss_maxpen;
		ss_maxpen << "Max Penetration: " << std::fixed << std::setprecision(1) << r.actual_penetration_depth << " meters";
		std::cout << "    .   Concrete Target         |         \\     /     "
			  << leftPad(ss_maxpen.str(), 29) << "|\n";
			  
		std::stringstream ss_dens; ss_dens << "(Density: " << target_density << " kg/m^3)";
		std::stringstream ss_depth_ft; ss_depth_ft << "(" << std::fixed << std::setprecision(1) << (r.actual_penetration_depth * 3.28084) << " feet deep into target)";
		std::cout << "    .   " << leftPad(ss_dens.str(), 24) << "|          \\___/      "
			  << leftPad(ss_depth_ft.str(), 28) << "|\n";
			  
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
			   
		std::stringstream ss_shock; 
		ss_shock << "(P_dyn = " << std::fixed << std::setprecision(1) << (r.dynamic_pressure / 1e9) << " GPa, Shock Damage = " << std::setprecision(0) << r.shock_damage_prob_percent << "%)";
		std::string fail_msg = (r.casing_failure ? "SURFACE DETONATION!" : "SHOCK FUZE FAILURE!");
		std::cout << "    *  [" << fail_msg << "] * |   "
			  << leftPad(ss_shock.str(), 37) << "|\n";
			  
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
		
		std::stringstream ss_dens2; ss_dens2 << "(Density: " << target_density2 << " kg/m^3)";
		std::cout << "    .   " << leftPad(ss_dens2.str(), 24) << "|   Max Penetration Depth:                    |\n";
		
		std::stringstream ss_d2; ss_d2 << "D = " << std::fixed << std::setprecision(2) << r.actual_penetration_depth << " m (" << (r.actual_penetration_depth * 3.28084) << " ft)";
		std::cout << "    .                           |   " << leftPad(ss_d2.str(), 41) << "|\n";
		
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
			   
		std::stringstream ss_pdyn3; ss_pdyn3 << "P_dyn = " << std::fixed << std::setprecision(2) << (r.dynamic_pressure / 1e9) << " GPa";
		std::cout << "     [ RIGID PENETRATION ]      |       ||       ||   "
			  << leftPad(ss_pdyn3.str(), 35) << "|\n";
			  
		std::stringstream ss_sd3; ss_sd3 << "(Shock Damage: " << std::fixed << std::setprecision(1) << r.shock_damage_prob_percent << "%)";
		std::cout << "     (Casing intact & payload   |       ||   *   ||   "
			  << leftPad(ss_sd3.str(), 35) << "|\n";
			  
		std::cout
			<< "      survives impact shock)    |       ||=======||                      "
			   "         |\n";
			   
		std::stringstream ss_rd3; ss_rd3 << "Rigid Depth: " << std::fixed << std::setprecision(1) << r.rigid_penetration << " m";
		std::cout << "                                |       ||   v   ||   "
			  << leftPad(ss_rd3.str(), 35) << "|\n";
			  
		std::cout << "    "
			     ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~\\.~.~.~./"
			     ".~.~.~.~.~.~.~.~.~.~.~.~.~.~. [Drilling Deep into Rock]\n";
		std::cout
			<< "    .   Concrete Target         |         \\     /                       "
			   "      |\n";
			   
		double target_density3 = target.layers.empty() ? 2500.0 : target.layers[0].density;
		
		std::stringstream ss_dens3; ss_dens3 << "(Density: " << target_density3 << " kg/m^3)";
		std::stringstream ss_reaches3; ss_reaches3 << "<-- Reaches " << std::fixed << std::setprecision(1) << r.actual_penetration_depth << " meters";
		std::cout << "    .   " << leftPad(ss_dens3.str(), 24) << "|          \\___/  "
			  << leftPad(ss_reaches3.str(), 28) << "|\n";
			  
		std::stringstream ss_ft3; ss_ft3 << "(" << std::fixed << std::setprecision(1) << (r.actual_penetration_depth * 3.28084) << " feet underground)";
		std::cout << "    .                           |              *      "
			  << leftPad(ss_ft3.str(), 28) << "|\n";
			  
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
	std::cout << std::left << std::setw(20) << "Scenario" 
		  << std::right << std::setw(14) << "Velocity" 
		  << std::right << std::setw(10) << "Mach" 
		  << std::right << std::setw(15) << "Dyn.Press" 
		  << std::right << std::setw(14) << "Depth (m)" 
		  << std::right << std::setw(12) << "Shock Dmg" 
		  << "  " << std::left << std::setw(30) << "Regime" 
		  << std::left << std::setw(20) << "Outcome" << "\n";
	std::cout << std::string(135, '-') << "\n";

	for (const auto& r : results) {
		std::stringstream vel_ss; vel_ss << std::fixed << std::setprecision(1) << r.velocity << " m/s";
		std::stringstream mach_ss; mach_ss << std::fixed << std::setprecision(1) << r.mach_number << "x";
		std::stringstream dyn_ss; dyn_ss << std::fixed << std::setprecision(2) << (r.dynamic_pressure / 1e9) << " GPa";
		std::stringstream dep_ss; dep_ss << std::fixed << std::setprecision(1) << r.actual_penetration_depth << " m";
		std::stringstream shk_ss; shk_ss << std::fixed << std::setprecision(0) << r.shock_damage_prob_percent << "%";

		std::cout << std::left << std::setw(20) << r.scenario_name 
			  << std::right << std::setw(14) << vel_ss.str() 
			  << std::right << std::setw(10) << mach_ss.str() 
			  << std::right << std::setw(15) << dyn_ss.str() 
			  << std::right << std::setw(14) << dep_ss.str() 
			  << std::right << std::setw(12) << shk_ss.str() 
			  << "  " << std::left << std::setw(30) << r.regime 
			  << std::left << std::setw(20) << r.outcome_summary << "\n";
			  
		// Full JSON frame for Node.js Automation Runner — matches Mongoose result.js schema
		std::string jsonOut = serializeResultToJson(r, proj, target);
		jsonOut = std::regex_replace(jsonOut, std::regex("\\b(nan|NaN)\\b"), "null");
		jsonOut = std::regex_replace(jsonOut, std::regex("\\b(inf|Infinity|-inf)\\b"), "null");
		std::cout << jsonOut << std::endl;
	}
	std::cout << std::string(135, '-') << "\n\n";

	// Print ASCII 3D cross sections for all simulated scenarios
	for (const auto& r : results) {
		printAscii3DVisualizer(r, proj, target);
	}
}

void generateHtml3DVisualizer(const std::vector<SimulationResult>& results,
			      const Projectile& proj,
			      const Target& target,
			      const std::string& basePath) {
	std::ifstream tpl(basePath + "/assets/versionOne/visualizer_template_light.html");
	if (!tpl.is_open()) {
		std::cerr
			<< "[!] Warning: HTML Visualizer template not found at " << basePath
			<< "/assets/versionOne/visualizer_template_light.html. Visualizer will not be generated.\n";
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

	// Generate scenario data — reuse serializeResultToJson() as the single source of truth
	std::stringstream data;
	for (size_t i = 0; i < results.size(); ++i) {
		const auto& r = results[i];
		std::string scenarioJson = serializeResultToJson(r, proj, target);
		scenarioJson = std::regex_replace(scenarioJson, std::regex("\\b(nan|NaN)\\b"), "null");
		scenarioJson = std::regex_replace(scenarioJson, std::regex("\\b(inf|Infinity|-inf)\\b"), "null");
		data << scenarioJson;
		if (i + 1 < results.size()) data << ",";
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

	std::string dataStr = data.str();
	replaceAll(html, "{{SCENARIO_BUTTONS}}", buttons.str());
	replaceAll(html, "/*{{SCENARIOS_DATA}}*/", dataStr);


	out << html;
	out.close();

	std::cout << "\n[+] Successfully generated 3D WebGL Interactive Visualizer: " << filename
		  << "\n";
	std::cout << "    -> Note: Modern browsers block local file security (CORS) by default.\n";
	std::cout << "    -> To view the 3D simulation scene, run a local web server in the directory:\n";
	std::cout << "       python -m http.server 8000\n";
	std::cout << "    -> Then open http://localhost:8000/3d_visualizer.html in your browser!\n";
}

} // namespace TelemetryExporter
