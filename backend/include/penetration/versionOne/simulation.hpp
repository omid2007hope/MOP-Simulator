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
	const double universalGasConstant = 8.31432;
	const double molarMassAir = 0.0289644;
	const double adiabaticIndexAir = 1.4;
	const double earthRadius = 6356766.0;
	const double SpeedOfSound = 343.0;
};




struct ImpactScenario {
	std::string name;
	double altitude_ft = 0.0;
	double velocity = 0.0;
	double flight_path_angle = 0.0;
	double obliquity_angle = 0.0;
	double angle_of_attack = 0.0;
};




// Atmospheric state at a given geometric altitude (US Standard Atmosphere 1976)
struct AtmosphereState {
	// 15 degree Celsius
	double temperature_K = 288.15;
	double pressure_Pa = 101325.0;
	double density_kgm3 = 1.225;
	double speed_of_sound_ms = 340.3;
};




struct Aircraft {
	std::string name;
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
	double area_moment_inertia = 0.0;
	double elastic_modulus = 0.0;
	double casing_wall_thickness = 0.0;
	double hugoniot_c0 = 0.0;
	double hugoniot_s = 0.0;
	double explosive_critical_energy = 0.0;
	double explosive_energy_j_per_kg = 0.0;
	// specific_heat determines how much the temperature changes for a given amount of energy.
	double specific_heat = 460.0;
	// The fixed temperature threshold where solid and liquid phases coexist in equilibrium. For water, this is 0°C, but it varies by material.
	double melting_point = 1800.0;
	// the amount of thermal energy required to change a unit mass of a substance from solid to liquid at its melting point without changing its temperature. This energy breaks the intermolecular bonds holding the solid structure together.
	double heat_of_fusion = 272000.0;
};




// Target layer specification
struct TargetLayer {
	std::string material_name;
	double thickness = 0.0;
	double density = 0.0;
	double compressive_strength = 0.0;
	double rebar_volume_fraction = 0.0;
	double rebar_yield_strength = 0.0;
	double pulverized_depth = 0.0;
	double hugoniot_c0 = 0.0;
	double hugoniot_s = 0.0;
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
	bool is_eroding = false;
	double dif = 0.0;
	double remaining_length = 0.0;
	double obliquity_deg = 0.0;
	double current_vx = 0.0;
	double current_vy = 0.0;
	double drag_coefficient = 0.0;
	double drag_force = 0.0;
	double guidance_pull = 0.0;
	double Up = 0.0;
	double Us = 0.0;
	double P_shock = 0.0;
	double transmitted_pressure = 0.0;
	double shock_energy = 0.0;
	double asymmetric_force = 0.0;
	double bending_moment = 0.0;
	double max_bending_stress = 0.0;
	double strain_rate = 0.0;
	double effective_strength = 0.0;
	double tunnel_force = 0.0;
	double interface_erosion_velocity = 0.0;
	double heat_rate = 0.0;
	double excess_heat = 0.0;
	double mass_loss = 0.0;
	double effective_linear_density = 0.0;
};




// Simulation results for a given scenario
struct SimulationResult {
	std::string scenario_name;
	double altitude_ft = 0.0;
	double velocity = 0.0;
	double mach_number = 0.0;
	double kinetic_energy = 0.0;
	double dynamic_pressure = 0.0;
	bool casing_failure = false;
	bool premature_detonation = false;
	double hydro_penetration = 0.0;
	double rigid_penetration = 0.0;
	double actual_penetration_depth = 0.0;
	double shock_damage_prob_percent = 0.0;
	bool explosive_charge_survives = true;
	bool is_kinetic_rod = false;

	std::string regime;
	std::string outcome_summary;
	bool erosion_occurred = false;
	double final_rod_length = 0.0;
	double erosion_length_lost = 0.0;
	double dynamic_increase_factor = 0.0;
	double bar_wave_speed = 0.0;
	double shock_pressure_gpa_peak = 0.0;
	double shock_pulse_duration_us = 0.0;
	double previous_strike_depth = 0.0;
	double cumulative_breach_depth = 0.0;
	double explosive_mass = 0.0;
	double explosion_scale = 0.0;
	double crater_wide_radius = 0.0;
	double crater_narrow_radius = 0.0;
	double camera_shake_magnitude = 0.0;
	double time_scale_pen = 0.0;
	double x_acceleration = 0.0;
	double y_acceleration = 0.0;
	double flight_path_angle = 0.0;
	double obliquity_angle = 0.0;
	double angle_of_attack = 0.0;
	double kinetic_shock_joules = 0.0;
	double total_explosive_yield = 0.0;
	double trim_deg = 0.0;
	double trim_rad = 0.0;
	double fpa_rad_corrected = 0.0;
	double area = 0.0;
	double boom_time = 0.0;
	double boom_alt = 0.0;
	double impact_velocity = 0.0;
	double impact_pitch = 0.0;
	double initial_shaft_depth = 0.0;
	std::vector<double> layer_bottom_depths;
	double critical_angle_threshold = 0.0;
	double average_density = 0.0;
	double aircraft_bomber_totalMass = 0.0;
	double aircraft_bomber_wingArea = 0.0;
	double aircraft_bomber_liftCurveSlope = 0.0;
	double cons_universalGasConstant = 0.0;
	double cons_molarMassAir = 0.0;
	double cons_adiabaticIndexAir = 0.0;
	double cons_earthRadius = 0.0;
	std::vector<TelemetryFrame> drop_frames;
	std::vector<TelemetryFrame> penetration_frames;
};




struct DropDeriv {
	double dv_x;
	double dv_y;
	double dy;
};




struct AngleSimulationResult {
	double trim_deg = 0.0;
	double trim_rad = 0.0;
	double fpa_rad_corrected = 0.0;
	double current_vx = 0.0;
	double current_vy = 0.0;
};




struct ShockWaveIgnitionResult {
	double dynamic_pressure = 0.0;
	double shock_pressure_gpa_peak = 0.0;
	double shock_pulse_duration_us = 0.0;
	double transmitted_pressure = 0.0;
	double shock_energy = 0.0;
	double velocity = 0.0;
	double kinetic_energy = 0.0;
	double shock_damage_prob_percent = 0.0;
	double actual_penetration_depth = 0.0;
	bool casing_failure = false;
	bool explosive_charge_survives = true;
	bool premature_detonation = false;
	std::string regime = "";
	std::string outcome_summary = "";
};




struct PostPenetrationCraterProfilingResult {

	double cumulative_breach_depth = 0.0;
	double actual_penetration_depth = 0.0;
	double rigid_penetration = 0.0;
	double dynamic_pressure = 0.0;
	double hydro_penetration = 0.0;
	double shock_damage_prob_percent = 0.0;
	double explosive_mass = 0.0;
	double explosion_scale = 0.0;
	double crater_wide_radius = 0.0;
	double crater_narrow_radius = 0.0;
	double camera_shake_magnitude = 0.0;
	bool is_kinetic_rod = false;
	bool explosive_charge_survives = true;
	bool premature_detonation = false;
	std::string regime = "";
	std::string outcome_summary = "";
};




struct ThermalMassAblationResult {
	double final_rod_length = 0.0;
	double erosion_length_lost = 0.0;
	bool casing_failure = false;
	bool should_break = false;
	std::string regime = "";
	std::string outcome_summary = "";
};

struct FramePackPayload {
	double t;
	double current_depth;
	double current_velocity;
	double groundSpeedOfSound;
	double dynamic_pressure;
	double acceleration;
	double current_temperature;
	bool erosion_active;
	double dynamic_increase_factor;
	double current_length;
	double obliquity_radians;
	double Up;
	double Us;
	double P_shock;
	double transmitted_pressure;
	double shock_energy;
	double asymmetric_force;
	double bending_moment;
	double max_bending_stress;
	double baseStrength;
	double baseDensity;
	double current_mass;
};

// ! Core impact simulator engine handling atmospheric trajectory and ground penetration physics
class ImpactSimulator {
	// ! purpose: encapsulated simulation engine that binds projectile properties, target strata, and physics models to compute trajectory and penetration results.
private:
	// ! proj payload comes from p (munition) in main.cpp - line 464 initialized in simulation.cpp - line 16
	Projectile proj;

	// ! target payload comes from t (object) in main.cpp - line 464 initialized in simulation.cpp - line 16
	Target target;

	// ! cons payload comes from c (cons) in main.cpp - line 464 initialized in simulation.cpp - line 16
	PhysicsConstants cons;




	// ! scenario, proj, res, impact_velocity, imp act_pitch and dt payloads come from simulation.cpp - line 668
	// ! simulateAtmosphericDrop(scenario, proj, res, impact_velocity, impact_pitch, dt);
	// ! purpose : simulates atmospheric free-fall phase using 2DOF integration (drag, gravity, US Standard Atmosphere 1976) to calculate terminal impact velocity and pitch angle.
	void simulateAtmosphericDrop(const ImpactScenario& scenario,
				     const Projectile& proj,
				     SimulationResult& res,
				     double& impact_velocity,
				     double& impact_pitch,
				     double dt);

	// ! scenario, res, impact_velocity, impact_pitch and dt payloads come from simulation.cpp - line 670
	// ! simulateGroundPenetration(scenario, res, impact_velocity, impact_pitch, dt);
	// ! purpose : simulates underground penetration phase into target layers using multi-phase physics (CEB-FIP DIF, Forrestal deceleration, WAPM rod erosion, Alekseevskii-Tate limits, and Walker-Wasley shock detonation assessment).
	void simulateGroundPenetration(const ImpactScenario& scenario,
				       SimulationResult& res,
				       double impact_velocity,
				       double impact_pitch,
				       double dt);

	AngleSimulationResult angleSimulation(double altitude,
					      double flightPathAngle,
					      double velocity,
					      double bombTotalMass);


	// Ground penetration
	ShockWaveIgnitionResult shockWaveIgnition(double currentVelocity,
						  double impactVelocity,
						  double rhoT,
						  double pShock,
						  double TAU);

	PostPenetrationCraterProfilingResult postPenetrationCraterProfiling(
		double currentDepth,
		double maxDynamicPressure,
		bool casingFailure,
		bool erosionOccurred,
		double kineticEnergy);

	ThermalMassAblationResult thermalMassAblation(bool erosionActive,
						      double& currentTemperature,
						      double& currentMass,
						      double& currentLength);

	TelemetryFrame buildPenetrationFrame(const FramePackPayload& payload);


public:
	// ! p, t and c payloads come from main.cpp - line 464
	// ! ImpactSimulator simulator(munition, object, cons);
	// ! purpose : initializes the simulation engine with specific projectile physical specs, multi-layer target configuration, and universal physics constants.
	ImpactSimulator(const Projectile& p, const Target& t, const PhysicsConstants& c);

	// ! Atmospheric drop
	static double impactShockwave(double totalMass, double velocityUponImpact);
	static double explosiveShockwave(double explosiveMass, double explosiveEnergy);


	// ! scenario payload comes from main.cpp - line 466.
	// ! results.push_back(simulator.simulate(sc));
	// ! purpose : executes the complete end-to-end simulation flow (atmospheric drop followed by ground penetration) for a given drop scenario and records telemetry frame data.
	SimulationResult simulate(const ImpactScenario& scenario);


	// ! target payload comes from main.cpp - line 478 and 480
	// ! TelemetryExporter::printReport(results, munition, simulator.getTarget());
	// ! purpose : provides read-only access to the internal target state, enabling reporting tools to inspect updated target layer properties (such as cumulative breach shaft depth after sequential strikes).
	const Target& getTarget() const {
		return target;
	}
};

#endif // SIMULATION_HPP
