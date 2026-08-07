// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#ifndef SIMULATION_HPP
#define SIMULATION_HPP

// packages
#include <numbers>
#include <string>
#include <vector>




struct PhysicsConstants {
	const double gravity = 9.81;
	const double PI = std::numbers::pi;
	const double frictionFactor = 0.1;
	// US Standard Atmosphere 1976 constants (atmosphere/drag/speed-of-sound model)
	const double universalGasConstant = 8.31432; // J/(mol*K)
	const double molarMassAir = 0.0289644;	     // kg/mol
	const double adiabaticIndexAir = 1.4;	     // dimensionless (gamma)
	const double earthRadius = 6356766.0; // m (geopotential reference radius, 45 deg lat)
};




// Scenario input definition
struct ImpactScenario {
	std::string name;
	double altitude_ft = 0.0;	// feet
	double velocity = 0.0;		// m/s
	double flight_path_angle = 0.0; // Degrees (0 = horizontal plane drop, 90 = vertical strike)
	double obliquity_angle = 0.0;	// Degrees (0 = perfectly perpendicular to ground)
	double angle_of_attack = 0.0;	// Degrees
};




// Atmospheric state at a given geometric altitude (US Standard Atmosphere 1976)
struct AtmosphereState {
	double temperature_K = 288.15;
	double pressure_Pa = 101325.0;
	double density_kgm3 = 1.225;
	double speed_of_sound_ms = 340.3;
};




struct Aircraft {
	double bomber_totalMass = 0.0;
	double bomber_wingArea = 0.0;
	double bomber_liftCurveSlope = 0.0;
};




// Projectile specification (e.g., GBU-57 MOP parameters)
struct Projectile {
	std::string name;
	double length = 0.0;
	double diameter = 0.0;
	double curvature_noseReduce = 0.0;
	double total_mass = 0.0;
	double explosive_mass = 0.0;
	double casing_density = 0.0;
	double yield_strength = 0.0;
	double specific_heat = 460.0;
	double melting_point = 1800.0;
	double heat_of_fusion = 272000.0;
	double area_moment_inertia = 0.0;
	double elastic_modulus = 0.0;
	double casing_wall_thickness = 0.0;
	double hugoniot_c0 = 0.0;
	double hugoniot_s = 0.0;
	double explosive_critical_energy = 0.0;
};




// Target layer specification
struct TargetLayer {
	std::string material_name;
	double thickness = 1.0;		      // meters
	double density = 2500.0;	      // kg/m^3
	double compressive_strength = 60.0e6; // Pascals
	double rebar_volume_fraction = 0.0;   // 0.0 to 1.0 (e.g. 0.02 for heavily reinforced)
	double rebar_yield_strength = 0.0;    // Pascals
	double pulverized_depth = 0.0;	      // meters (depth of crater from previous strikes)

	// Hugoniot EOS for shock impedance matching (Walker-Wasley shock initiation)
	double hugoniot_c0 = 3200.0; // m/s (bulk sound speed, Us = C0 + S*Up)
	double hugoniot_s = 1.9;     // dimensionless (Hugoniot slope)
};

// Target material specification
struct Target {
	std::string name;
	std::vector<TargetLayer> layers;

	void pulverizeDepth(double breachDepth) {
		double currentDepthAcc = 0.0;
		for (auto& layer : layers) {
			if (breachDepth > currentDepthAcc) {
				layer.pulverized_depth = std::max(
					layer.pulverized_depth,
					std::min(layer.thickness, breachDepth - currentDepthAcc));
			}
			currentDepthAcc += layer.thickness;
		}
	}
};




struct TelemetryFrame {
	double time = 0.0;
	double altitude = 0.0;
	double depth = 0.0;
	double velocity = 0.0;
	double mach = 0.0;
	double dynamic_pressure = 0.0;
	bool is_sonic_boom = false;
	double heat = 0.0;
	double g_force = 0.0;
	double pitch_rad = 0.0;

	// Penetration-phase physics telemetry (Phase 3/4 two-phase Forrestal + WAPM)
	bool is_eroding = false;       // true once the Walker-Anderson erosion regime is active
	double dif = 1.0;	       // CEB-FIP Dynamic Increase Factor at this instant
	double remaining_length = 0.0; // meters (rigid/eroding rod length at this instant)
	double obliquity_deg = 0.0;    // degrees (instantaneous obliquity, for trajectory bending)
};

// Simulation results for a given scenario
struct SimulationResult {
	std::string scenario_name;
	double altitude_ft = 0.0;
	double velocity = 0.0;
	double mach_number = 0.0;
	double kinetic_energy = 0.0;   // Joules
	double dynamic_pressure = 0.0; // Pascals
	bool casing_failure = false;
	bool premature_detonation = false;
	double hydro_penetration = 0.0;		// meters (Alekseevskii-Tate limit)
	double rigid_penetration = 0.0;		// meters (Work-energy concrete deceleration model)
	double actual_penetration_depth = 0.0;	// meters (Selected depth based on regime)
	double shock_damage_prob_percent = 0.0; // 0% to 100% chance of explosive failure from shock
	bool explosive_charge_survives = true;	// true if charge holds intact without shock damage
	bool is_kinetic_rod = false;		// true if explosive_mass == 0
	std::string regime;
	std::string outcome_summary;

	// Two-phase penetration & Walker-Anderson erosion (WAPM) telemetry
	bool erosion_occurred = false; // true if hydrodynamic pressure ever exceeded casing yield
	double final_rod_length = 0.0; // meters (remaining rigid length after erosion)
	double erosion_length_lost = 0.0;     // meters (proj.length - final_rod_length)
	double dynamic_increase_factor = 1.0; // last-evaluated CEB-FIP DIF on the target strength
	double bar_wave_speed = 0.0;	      // m/s (WAPM elastic bar wave speed, sqrt(E/rho_p))

	// Walker-Wasley shock initiation telemetry (Hugoniot impedance matching)
	double shock_pressure_gpa_peak =
		0.0; // GPa (peak transmitted shock pressure at any interface)
	double shock_pulse_duration_us =
		0.0; // microseconds (shock transit time through casing wall)

	// Multi-bomb salvo & Operation Midnight Hammer sequential shaft telemetry
	double previous_strike_depth = 0.0; // meters (breached shaft entry depth from prior bombs)
	double cumulative_breach_depth =
		0.0; // meters (total accumulated shaft depth after this bomb)

	// Visualization Data
	double explosive_mass = 0.0;
	double explosion_scale = 1.0;
	double crater_wide_radius = 4.5;
	double crater_narrow_radius = 1.0;
	double camera_shake_magnitude = 0.0;
	double time_scale_pen = 0.02;

	double x_acceleration = 0.0;
	double y_acceleration = 0.0;

	std::vector<TelemetryFrame> drop_frames;
	std::vector<TelemetryFrame> penetration_frames;
};

struct DropDeriv {
	double dv_x;
	double dv_y;
	double dy;
};


// ! Core impact simulator engine handling atmospheric trajectory and ground penetration physics
class ImpactSimulator {

private:
	// ! proj payload comes from p (munition) in main.cpp - line 464 initialized in simulation.cpp - line 16
	Projectile proj;

	// ! target payload comes from t (object) in main.cpp - line 464 initialized in simulation.cpp - line 16
	Target target;

	// ! cons payload comes from c (cons) in main.cpp - line 464 initialized in simulation.cpp - line 16
	PhysicsConstants cons;

	// ! scenario, proj, res, impact_velocity, impact_pitch and dt payloads come from simulation.cpp - line 668
	// ! simulateAtmosphericDrop(scenario, proj, res, impact_velocity, impact_pitch, dt);
	void simulateAtmosphericDrop(const ImpactScenario& scenario,
				     const Projectile& proj,
				     SimulationResult& res,
				     double& impact_velocity,
				     double& impact_pitch,
				     double dt);

	// ! scenario, res, impact_velocity, impact_pitch and dt payloads come from simulation.cpp - line 670
	// ! simulateGroundPenetration(scenario, res, impact_velocity, impact_pitch, dt);
	void simulateGroundPenetration(const ImpactScenario& scenario,
				       SimulationResult& res,
				       double impact_velocity,
				       double impact_pitch,
				       double dt);

public:
	// ! p, t and c payloads come from main.cpp - line 464
	// ! ImpactSimulator simulator(munition, object, cons);
	ImpactSimulator(const Projectile& p, const Target& t, const PhysicsConstants& c);

	// ! scenario payload comes from main.cpp - line 466.
	// ! results.push_back(simulator.simulate(sc));
	SimulationResult simulate(const ImpactScenario& scenario);


	// ! target payload comes from main.cpp - line 478 and 480
	// ! TelemetryExporter::printReport(results, munition, simulator.getTarget());
	const Target& getTarget() const {
		return target;
	}
};

#endif // SIMULATION_HPP
