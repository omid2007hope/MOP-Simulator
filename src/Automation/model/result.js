// © 2026 Omid Teimory. All rights reserved.
// Signature: OmidTeimory-2026

const mongoose = require('mongoose');

const simulationResultSchema = new mongoose.Schema(
	{
		// Scenario Identification & Outcomes
		name: { type: String, required: true, trim: true },
		velocity: { type: Number, required: true },
		mach: { type: Number, required: true },
		energy: { type: Number, required: true }, // kinetic_energy (GJ)
		pressurvives: { type: Boolean, default: true }, // explosive_charge_survives
		is_kinetic: { type: Boolean, default: false }, // is_kinetic_rod
		regime: { type: String, default: '' },
		summary: { type: String, default: '' },

		// Projectile Parameters
		proj_name: { type: String, default: '' },
		proj_length: { type: Number, default: 0 },
		proj_diameter: { type: Number, default: 0 },
		proj_total_mass: { type: Number, default: 0 },
		proj_curvature_noseReduce: { type: Number, default: 0 },
		proj_casing_density: { type: Number, default: 0 },
		proj_casing_wall_thickness: { type: Number, default: 0 },
		proj_area_moment_inertia: { type: Number, default: 0 },
		proj_elastic_modulus: { type: Number, default: 0 },
		proj_hugoniot_c0: { type: Number, default: 0 },
		proj_hugoniot_s: { type: Number, default: 0 },
		proj_explosive_critical_energy: { type: Number, default: 0 },
		proj_explosive_energy_j_per_kg: { type: Number, default: 0 },
		proj_specific_heat: { type: Number, default: 0 },
		proj_melting_point: { type: Number, default: 0 },
		proj_heat_of_fusion: { type: Number, default: 0 },

		// Target & Scenario Angles
		target_name: { type: String, default: '' },
		flight_path_angle: { type: Number, default: 0 },
		obliquity_angle: { type: Number, default: 0 },
		angle_of_attack: { type: Number, default: 0 },

		// Physical Constants
		cons_gravity: { type: Number, default: 0 },
		cons_pi: { type: Number, default: 0 },
		cons_friction_factor: { type: Number, default: 0 },
		cons_speed_of_sound: { type: Number, default: 0 },
		cons_universalGasConstant: { type: Number, default: 0 },
		cons_molarMassAir: { type: Number, default: 0 },
		cons_adiabaticIndexAir: { type: Number, default: 0 },
		cons_earthRadius: { type: Number, default: 0 },

		// Explosion, Crater & Detonation Dynamics
		explosive_mass: { type: Number, default: 0 },
		explosion_scale: { type: Number, default: 0 },
		crater_wide_radius: { type: Number, default: 0 },
		crater_narrow_radius: { type: Number, default: 0 },
		camera_shake_magnitude: { type: Number, default: 0 },
		time_scale_pen: { type: Number, default: 0 },
		total_explosive_yield: { type: Number, default: 0 },
		premature_detonation: { type: Boolean, default: false },

		// Erosion & Rod Mechanics
		erosion_occurred: { type: Boolean, default: false },
		final_rod_length: { type: Number, default: 0 },
		erosion_length_lost: { type: Number, default: 0 },

		// Dynamic & Shock Wave Analysis
		dynamic_increase_factor: { type: Number, default: 0 },
		bar_wave_speed: { type: Number, default: 0 },
		shock_pressure_gpa_peak: { type: Number, default: 0 },
		shock_pulse_duration_us: { type: Number, default: 0 },
		previous_strike_depth: { type: Number, default: 0 },
		cumulative_breach_depth: { type: Number, default: 0 },
		kinetic_shock_joules: { type: Number, default: 0 },

		// Atmospheric & Flight Kinematics
		altitude_ft: { type: Number, default: 0 },
		x_acceleration: { type: Number, default: 0 },
		y_acceleration: { type: Number, default: 0 },
		trim_deg: { type: Number, default: 0 },
		trim_rad: { type: Number, default: 0 },
		fpa_rad_corrected: { type: Number, default: 0 },
		area: { type: Number, default: 0 },
		boom_time: { type: Number, default: 0 },
		boom_alt: { type: Number, default: 0 },
		impact_velocity: { type: Number, default: 0 },
		impact_pitch: { type: Number, default: 0 },
		initial_shaft_depth: { type: Number, default: 0 },
		critical_angle_threshold: { type: Number, default: 0 },
		average_density: { type: Number, default: 0 },

		// Aircraft / Bomber Platform Parameters
		aircraft_bomber_totalMass: { type: Number, default: 0 },
		aircraft_bomber_wingArea: { type: Number, default: 0 },
		aircraft_bomber_liftCurveSlope: { type: Number, default: 0 },

		// Target Layers & Structural Geometry
		target_layers: [targetLayerSchema],
		layer_bottom_depths: [{ type: Number }],

		// High-Frequency Drop & Penetration Telemetry Frames
		drop_frames: [dropFrameSchema],
		pen_frames: [penetrationFrameSchema],
	},
	{
		timestamps: true,
	},
);

module.exports = mongoose.model('SimulationResultSchema', simulationResultSchema);
