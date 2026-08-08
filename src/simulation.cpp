// Copyright (c) 2026 Omid Teimory. All Rights Reserved

// packages
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

// files
#include "default.hpp"
#include "environment_physics.hpp"
#include "simulation.hpp"


ImpactSimulator::ImpactSimulator(const Projectile& p, const Target& t, const PhysicsConstants& c)
    : proj(p), target(t), cons(c) {}

double ImpactSimulator::impactShockwave(double totalMass, double velocityUponImpact) {
	double kineticShock = 0.5 * totalMass * std::pow(velocityUponImpact, 2);
	return kineticShock;
}

double ImpactSimulator::explosiveShockwave(double explosiveMass, double explosiveEnergy) {

	// Because the explosion is buried in dense rock (impedance matching),
	// energy is not lost to the atmosphere (which would normally be 10-50% efficiency).
	// ! NEEDS TO BE CALCULATED ! IMPORTANT ASF !
	double couplingEfficiency = 1.0; // "nearly 100%"


	double explosiveShock = explosiveMass * explosiveEnergy * couplingEfficiency;
	return explosiveShock;
}

void ImpactSimulator::simulateAtmosphericDrop(const ImpactScenario& scenario,
					      const Projectile& proj,
					      SimulationResult& res,
					      double& impact_velocity,
					      double& impact_pitch,
					      double dt) {
	// ! ShockWave
	// ! ShockWave
	impactShockwave(proj.total_mass, res.velocity);
	explosiveShockwave(proj.explosive_mass, proj.explosive_energy_j_per_kg);
	// ! ShockWave
	// ! ShockWave

	(void)dt; // Suppress unused parameter warning


	AtmosphereState atmos =
		EnvironmentPhysics::standardAtmosphere(scenario.altitude_ft / 3.28084, cons);

	double fpa_rad = scenario.flight_path_angle * cons.PI / 180.0;

	double trim_deg = EnvironmentPhysics::flightControlTrim(
		scenario.flight_path_angle,
		scenario.velocity,
		proj.total_mass,
		B2_Sprit_Strategic_Bomber.bomber_liftCurveSlope,
		B2_Sprit_Strategic_Bomber.bomber_wingArea,
		atmos.density_kgm3,
		cons);

	double trim_rad = trim_deg * cons.PI / 180.0;
	double fpa_rad_corrected = fpa_rad - trim_rad;

	double current_altitude = scenario.altitude_ft;
	double current_vx = scenario.velocity * std::cos(fpa_rad_corrected);
	double current_vy = scenario.velocity * std::sin(fpa_rad_corrected);
	double current_velocity = scenario.velocity;

	double area = cons.PI * std::pow(proj.diameter / 2.0, 2);
	double CRH = (proj.diameter > 0.0) ? (proj.curvature_noseReduce / proj.diameter) : 3.0;
	double Caliber_Radius_Head = (CRH > 0.0) ? CRH : 3.0;
	double dragCoefficient =
		(8.0 * Caliber_Radius_Head - 1.0) / (24.0 * std::pow(Caliber_Radius_Head, 2));

	if (current_altitude > 0.0) {
		std::cout << "\n--- Simulating Atmospheric Drop from " << current_altitude
			  << " ft ---\n";
		double dt_drop = 0.01;
		double next_print_altitude = current_altitude - 5000.0;
		bool sonic_boom_triggered = false;
		double t_drop = 0.0;
		int drop_frame_counter = 0;

		double y_m = current_altitude / 3.28084;

		while (current_altitude > 0.0) {
			auto calc_derivs = [&](double alt_m, double vx, double vy) -> DropDeriv {
				AtmosphereState atm =
					EnvironmentPhysics::standardAtmosphere(alt_m, cons);
				double v_mag = std::hypot(vx, vy);
				double mach = v_mag / atm.speed_of_sound_ms;
				double cd = EnvironmentPhysics::getMachDependentDrag(
					mach, dragCoefficient, proj, cons);
				double f = 0.5 * atm.density_kgm3 * v_mag * v_mag * cd * area;
				DropDeriv d;
				if (v_mag > 0.0) {
					d.dv_x = -(f / proj.total_mass) * (vx / v_mag);
					d.dv_y =
						cons.gravity - (f / proj.total_mass) * (vy / v_mag);

					double guidance_pull = 1.5 * cons.gravity;
					double guidance_accel =
						(vx > 0.0) ? -guidance_pull : guidance_pull;
					if (std::abs(vx) < 5.0) {
						guidance_accel = -vx * 1.5;
					}
					d.dv_x += guidance_accel;
				} else {
					d.dv_x = 0.0;
					d.dv_y = cons.gravity;
				}
				d.dy = -vy;
				return d;
			};

			DropDeriv k1 = calc_derivs(y_m, current_vx, current_vy);
			DropDeriv k2 = calc_derivs(y_m + 0.5 * dt_drop * k1.dy,
						   current_vx + 0.5 * dt_drop * k1.dv_x,
						   current_vy + 0.5 * dt_drop * k1.dv_y);
			DropDeriv k3 = calc_derivs(y_m + 0.5 * dt_drop * k2.dy,
						   current_vx + 0.5 * dt_drop * k2.dv_x,
						   current_vy + 0.5 * dt_drop * k2.dv_y);
			DropDeriv k4 = calc_derivs(y_m + dt_drop * k3.dy,
						   current_vx + dt_drop * k3.dv_x,
						   current_vy + dt_drop * k3.dv_y);

			double prev_y_m = y_m;
			double prev_vx = current_vx;
			double prev_vy = current_vy;

			current_vx +=
				(dt_drop / 6.0) * (k1.dv_x + 2 * k2.dv_x + 2 * k3.dv_x + k4.dv_x);
			current_vy +=
				(dt_drop / 6.0) * (k1.dv_y + 2 * k2.dv_y + 2 * k3.dv_y + k4.dv_y);
			y_m += (dt_drop / 6.0) * (k1.dy + 2 * k2.dy + 2 * k3.dy + k4.dy);
			current_velocity = std::hypot(current_vx, current_vy);

			if (y_m < 0.0) {
				double fraction = prev_y_m / (prev_y_m - y_m);
				current_vx = prev_vx + fraction * (current_vx - prev_vx);
				current_vy = prev_vy + fraction * (current_vy - prev_vy);
				y_m = 0.0;
				t_drop = t_drop - dt_drop + fraction * dt_drop;
				current_velocity = std::hypot(current_vx, current_vy);
			}

			current_altitude = y_m * 3.28084;
			AtmosphereState current_atm =
				EnvironmentPhysics::standardAtmosphere(y_m, cons);
			double current_density = current_atm.density_kgm3;

			bool is_sonic_boom_frame = false;
			// Use the velocity and altitude at the *start* of the frame if this is the frame it breaks Mach 1
			if (current_velocity >= current_atm.speed_of_sound_ms &&
			    !sonic_boom_triggered) {
				sonic_boom_triggered = true;
				is_sonic_boom_frame = true;
				
				// To eliminate the 1-tick lag, we print the state exactly as it was when the threshold was crossed.
				// Since we stepped over it, we interpolate to the exact moment. But for simplicity and zero-lag,
				// if t_drop <= dt_drop, it means it started supersonic.
				double boom_time = (t_drop <= dt_drop) ? 0.0 : t_drop;
				double boom_alt = (t_drop <= dt_drop) ? scenario.altitude_ft : current_altitude;
				
				std::cout << "  >>> [SONIC BOOM] Mach 1 exceeded at T+ "
					  << std::fixed << std::setprecision(2) << boom_time
					  << "s (Altitude: " << std::setprecision(0)
					  << boom_alt
					  << " ft | Density: " << std::setprecision(3)
					  << current_density << " kg/m^3) <<<\n";
			}

			if (drop_frame_counter++ % 10 == 0 || is_sonic_boom_frame) {
				TelemetryFrame frame;
				frame.time = t_drop;
				frame.altitude = current_altitude / 3.28084;
				frame.depth = -frame.altitude;
				frame.velocity = current_velocity;
				frame.mach = current_velocity / current_atm.speed_of_sound_ms;
				frame.is_sonic_boom = is_sonic_boom_frame;
				frame.pitch_rad =
					std::atan2(current_vx, std::max(0.001, current_vy));
				res.drop_frames.push_back(frame);
			}

			if (current_altitude <= next_print_altitude && current_altitude > 0.0) {
				double cd = EnvironmentPhysics::getMachDependentDrag(
					current_velocity / current_atm.speed_of_sound_ms,
					dragCoefficient,
					proj,
					cons);
				double drag_force = 0.5 * current_atm.density_kgm3 *
						    std::pow(current_velocity, 2) * cd * area;
				double acceleration = cons.gravity - (drag_force / proj.total_mass);
				double g_force = acceleration / cons.gravity;
				std::cout << "  [Drop T+ " << std::fixed << std::setprecision(1)
					  << t_drop << "s] Alt: " << std::setprecision(0)
					  << current_altitude
					  << " ft | Vel: " << std::setprecision(1)
					  << current_velocity
					  << " m/s | Accel: " << std::setprecision(2) << g_force
					  << " G | Density: " << std::setprecision(3)
					  << current_density << " kg/m^3\n";
				next_print_altitude -= 5000.0;
			}
			t_drop += dt_drop;
		}

		std::cout << "  [Impact T+ " << std::fixed << std::setprecision(2) << t_drop
			  << "s] Alt: 0 ft | Impact Velocity: " << std::setprecision(1)
			  << current_velocity << " m/s (Vx: " << current_vx
			  << ", Vy: " << current_vy << ")\n";
		std::cout << "--------------------------------------------------------\n\n";
	}

	impact_velocity = current_velocity;
	impact_pitch = std::atan2(current_vx, std::max(0.001, current_vy));
}

void ImpactSimulator::simulateGroundPenetration(const ImpactScenario& scenario,
						SimulationResult& res,
						double impact_velocity,
						double impact_pitch,
						double dt) {
	double target_obliquity_radians = scenario.obliquity_angle * cons.PI / 180.0;
	double obliquity_radians = target_obliquity_radians + impact_pitch;
	double angleOfAttack_radians = scenario.angle_of_attack * cons.PI / 180.0;

	double current_velocity = impact_velocity;
	double current_depth = 0.0;
	double current_mass = proj.total_mass;
	double current_temperature = 300.0;
	double current_length = proj.length;
	double area = cons.PI * std::pow(proj.diameter / 2.0, 2);

	double max_dynamic_pressure = 0.0;
	bool erosion_active = false;
	double bar_wave_speed = std::sqrt(proj.elastic_modulus / proj.casing_density);
	res.bar_wave_speed = bar_wave_speed;

	double initial_shaft_depth = 0.0;
	for (const auto& layer : target.layers) {
		if (layer.pulverized_depth > 0) {
			initial_shaft_depth += std::min(layer.thickness, layer.pulverized_depth);
		}
	}
	res.previous_strike_depth = initial_shaft_depth;
	if (initial_shaft_depth > 0) {
		current_depth = initial_shaft_depth;
		std::cout
			<< "  [SEQUENTIAL SALVO STRIKE] Entering pre-existing breached shaft depth: "
			<< initial_shaft_depth << " m\n";
	}

	std::vector<double> layer_bottom_depths;
	double fullDepth = 0.0;
	for (const auto& layer : target.layers) {
		fullDepth += layer.thickness;
		layer_bottom_depths.push_back(fullDepth);
	}

	size_t current_layer_idx = 0;
	while (current_layer_idx < layer_bottom_depths.size() &&
	       current_depth >= layer_bottom_depths[current_layer_idx]) {
		current_layer_idx++;
	}
	size_t last_layer_idx = current_layer_idx;
	double next_print_depth = std::floor(current_depth) + 1.0;
	int pen_frame_counter = 0;

	if (current_layer_idx < target.layers.size()) {
		std::cout << "--- Ground Penetration Commenced ---\n";
		std::cout << "  [LAYER BREACH] Entering layer: "
			  << target.layers[current_layer_idx].material_name << "\n";
	}

	double critical_angle_threshold = 65.0 * cons.PI / 180.0;
	if (current_velocity < 200.0) {
		critical_angle_threshold = 50.0 * cons.PI / 180.0;
	}

	if ((obliquity_radians + angleOfAttack_radians) >= critical_angle_threshold) {
		res.casing_failure = true;
		res.regime = "Ricochet";
		res.outcome_summary = "Projectile deflected off target surface.";
		res.actual_penetration_depth = 0.0;
		return;
	}



	const double groundSpeedOfSound =
		EnvironmentPhysics::standardAtmosphere(0.0, cons).speed_of_sound_ms;
	res.mach_number = current_velocity / groundSpeedOfSound;

	double rho_t = target.layers.empty() ? 2500.0 : target.layers[0].density;
	double rho_p = proj.casing_density > 0 ? proj.casing_density : 7800.0;
	double Up = current_velocity / (1.0 + std::sqrt(rho_t / rho_p));
	double c0 = proj.hugoniot_c0 > 0 ? proj.hugoniot_c0 : 4570.0;
	double s_coef = proj.hugoniot_s > 0 ? proj.hugoniot_s : 1.49;
	double Us = c0 + s_coef * Up;
	double P_shock = rho_p * Us * Up;
	double wall_thick = proj.casing_wall_thickness > 0 ? proj.casing_wall_thickness : 0.05;
	double tau = (2.0 * wall_thick) / c0;

	// Set initial dynamic pressure on impact so it is never 0.00 GPa
	max_dynamic_pressure = 0.5 * rho_t * current_velocity * current_velocity;
	res.dynamic_pressure = max_dynamic_pressure;

	// Approximate shock transmission coefficient from steel casing to explosive fill
	// Z_steel = ~35 MRayls, Z_explosive = ~5 MRayls. T = 2*Z2/(Z1+Z2) =~ 0.25
	double shock_transmission_coef = 0.25;
	double transmitted_pressure = P_shock * shock_transmission_coef;

	res.shock_pressure_gpa_peak = transmitted_pressure / 1.0e9;
	res.shock_pulse_duration_us = tau * 1.0e6;

	double shock_energy = transmitted_pressure * transmitted_pressure * tau;
	res.velocity = impact_velocity;
	res.kinetic_energy = 0.5 * proj.total_mass * std::pow(impact_velocity, 2);

	if (proj.explosive_mass > 0.0 && proj.explosive_critical_energy > 0.0) {
		if (shock_energy >= proj.explosive_critical_energy) {
			res.casing_failure = true;
			res.explosive_charge_survives = false;
			res.premature_detonation = true;
			res.shock_damage_prob_percent = 100.0;
			res.regime = "Shock Initiation (Walker-Wasley)";
			res.outcome_summary = "Premature detonation triggered by Hugoniot impact shock.";
			res.actual_penetration_depth = 0.0;
			return;
		}
	}

	double t = 0.0;

	struct PenDeriv {
		double dv = 0.0;
		double dz = 0.0;
		double dtheta = 0.0;
		double dT = 0.0;
		double dL = 0.0;
	};

	while (current_velocity > 0.0 && !res.casing_failure && current_depth < fullDepth) {
		while (current_layer_idx < layer_bottom_depths.size() &&
		       current_depth >= layer_bottom_depths[current_layer_idx]) {
			current_layer_idx++;
		}

		if (current_layer_idx >= target.layers.size()) {
			res.regime = "Target Perforated";
			res.outcome_summary = "Projectile completely pierced all target layers.";
			break;
		}

		if (current_layer_idx != last_layer_idx) {
			last_layer_idx = current_layer_idx;
			std::cout << "  [LAYER BREACH] Pierced into layer: "
				  << target.layers[current_layer_idx].material_name << "\n";
		}

		const auto& layer = target.layers[current_layer_idx];
		double layerEntryDepth =
			(current_layer_idx == 0) ? 0.0 : layer_bottom_depths[current_layer_idx - 1];

		double squaredVelocity = current_velocity * current_velocity;
		double baseStrength = layer.compressive_strength +
				      (layer.rebar_yield_strength * layer.rebar_volume_fraction);
		double baseDensity = layer.density;

		if ((current_depth - layerEntryDepth) < layer.pulverized_depth) {
			baseStrength = 5.0e6;
			baseDensity = layer.density * 0.7;
		}

		double dynamic_pressure = 0.5 * baseDensity * squaredVelocity;
		if (dynamic_pressure > max_dynamic_pressure) {
			max_dynamic_pressure = dynamic_pressure;
		}

		if (!erosion_active && proj.yield_strength > 0.0 &&
		    dynamic_pressure >= proj.yield_strength) {
			erosion_active = true;
			res.erosion_occurred = true;
			std::cout
				<< "  [WAPM EROSION ONSET] Hydrodynamic pressure exceeded casing yield at Depth: "
				<< current_depth << " m | Velocity: " << current_velocity
				<< " m/s\n";
		}

		double asymmetric_force = 0.0;
		if (obliquity_radians > 0.0 || angleOfAttack_radians > 0.0) {
			asymmetric_force = (0.5 * baseDensity * squaredVelocity * area) *
					   std::sin(obliquity_radians + angleOfAttack_radians);
			double bending_moment = asymmetric_force * (proj.length / 2.0);

			double max_bending_stress = 0.0;
			if (proj.area_moment_inertia > 0) {
				max_bending_stress = (bending_moment * (proj.diameter / 2.0)) /
						     proj.area_moment_inertia;
			}

			if (proj.yield_strength > 0.0 && max_bending_stress > proj.yield_strength) {
				res.casing_failure = true;
				res.regime = "Structural Failure (J-Hook/Snap)";
				res.outcome_summary =
					"Bending moments exceeded casing yield strength.";
				break;
			}
		}

		auto derivative =
			[&](double v, double z, double theta, double T, double L, double m)
			-> PenDeriv {
			(void)T; // Suppress unused parameter warning
			PenDeriv d;
			double vSq = v * v;
			double strain_rate = std::fabs(v) / std::max(0.01, proj.diameter);
			double dif = EnvironmentPhysics::computeDIF(strain_rate, baseStrength);
			res.dynamic_increase_factor = dif;
			double effective_strength = baseStrength * dif;

			double lateral_force = 0.0;
			if (theta > 0.0 || angleOfAttack_radians > 0.0) {
				lateral_force = (0.5 * baseDensity * vSq * area) *
						std::sin(theta + angleOfAttack_radians);
			}
			double safeMass = std::max(0.001, m);
			double gravity_component = cons.gravity * std::cos(theta);

			if (!erosion_active) {
				double fc_mpa = std::max(0.001, effective_strength / 1.0e6);
				double S = 82.6 * std::pow(fc_mpa, -0.544);

				double CRH_val =
					(proj.diameter > 0.0)
						? (proj.curvature_noseReduce / proj.diameter)
						: 3.0;
				double dragCoef =
					(8.0 * ((CRH_val > 0.0) ? CRH_val : 3.0) - 1.0) /
					(24.0 * std::pow(((CRH_val > 0.0) ? CRH_val : 3.0), 2));

				double tunnelForce = area * (S * effective_strength +
							     dragCoef * baseDensity * vSq);
				double craterDepthLimit = 2.0 * proj.diameter;
				double zLocal = z - layerEntryDepth;
				double axialForce =
					(craterDepthLimit > 0.0 && zLocal < craterDepthLimit)
						? tunnelForce *
							  std::clamp(zLocal / craterDepthLimit,
								     0.0,
								     1.0)
						: tunnelForce;

				d.dv = gravity_component - (axialForce / safeMass);
				d.dz = v * std::cos(theta);
				d.dL = 0.0;

				double heat_rate =
					(axialForce * cons.frictionFactor) * std::fabs(v);
				d.dT = heat_rate / (safeMass * proj.specific_heat);
			} else {
				double u = EnvironmentPhysics::solveInterfaceVelocity(
					v,
					proj.casing_density,
					baseDensity,
					proj.yield_strength,
					effective_strength);
				double Le = std::max(0.01, L);
				d.dv = -(proj.yield_strength / (proj.casing_density * Le)) *
					       (1.0 + (v - u) / bar_wave_speed) +
				       gravity_component;
				d.dz = u * std::cos(theta);
				d.dL = -(v - u);

				double erosion_heat_rate = 0.5 * baseDensity * (v - u) * (v - u) *
							   area * std::fabs(v - u);
				d.dT = erosion_heat_rate / (safeMass * proj.specific_heat);
			}

			if (v > 0.1) {
				d.dtheta = (lateral_force / safeMass) / v;
			}

			return d;
		};

		auto get_mass = [&](double L_eval) {
			return erosion_active
				       ? (proj.total_mass / proj.length) * std::max(0.0, L_eval)
				       : current_mass;
		};

		PenDeriv k1 = derivative(current_velocity,
					 current_depth,
					 obliquity_radians,
					 current_temperature,
					 current_length,
					 get_mass(current_length));
		PenDeriv k2 = derivative(current_velocity + 0.5 * dt * k1.dv,
					 current_depth + 0.5 * dt * k1.dz,
					 obliquity_radians + 0.5 * dt * k1.dtheta,
					 current_temperature + 0.5 * dt * k1.dT,
					 current_length + 0.5 * dt * k1.dL,
					 get_mass(current_length + 0.5 * dt * k1.dL));
		PenDeriv k3 = derivative(current_velocity + 0.5 * dt * k2.dv,
					 current_depth + 0.5 * dt * k2.dz,
					 obliquity_radians + 0.5 * dt * k2.dtheta,
					 current_temperature + 0.5 * dt * k2.dT,
					 current_length + 0.5 * dt * k2.dL,
					 get_mass(current_length + 0.5 * dt * k2.dL));
		PenDeriv k4 = derivative(current_velocity + dt * k3.dv,
					 current_depth + dt * k3.dz,
					 obliquity_radians + dt * k3.dtheta,
					 current_temperature + dt * k3.dT,
					 current_length + dt * k3.dL,
					 get_mass(current_length + dt * k3.dL));

		double acceleration = k1.dv;

		current_velocity += (dt / 6.0) * (k1.dv + 2 * k2.dv + 2 * k3.dv + k4.dv);
		current_depth += (dt / 6.0) * (k1.dz + 2 * k2.dz + 2 * k3.dz + k4.dz);
		obliquity_radians +=
			(dt / 6.0) * (k1.dtheta + 2 * k2.dtheta + 2 * k3.dtheta + k4.dtheta);
		current_temperature += (dt / 6.0) * (k1.dT + 2 * k2.dT + 2 * k3.dT + k4.dT);
		current_length += (dt / 6.0) * (k1.dL + 2 * k2.dL + 2 * k3.dL + k4.dL);

		if (!erosion_active) {
			if (current_temperature > proj.melting_point) {
				double excess_temp = current_temperature - proj.melting_point;
				double excess_heat =
					excess_temp * current_mass * proj.specific_heat;

				if (excess_heat > 0 && proj.heat_of_fusion > 0) {
					double mass_loss = excess_heat / proj.heat_of_fusion;
					current_mass -= mass_loss;
					current_temperature = proj.melting_point;

					if (current_mass < 0.1 * proj.total_mass) {
						res.casing_failure = true;
						res.regime = "Thermal Destruction";
						res.outcome_summary =
							"Projectile completely ablated.";
						break;
					}
				}
			}
		} else {
			current_length = std::max(0.0, current_length);
			double effective_linear_density = proj.total_mass / proj.length;
			current_mass = effective_linear_density * current_length;
			res.final_rod_length = current_length;
			res.erosion_length_lost = proj.length - current_length;

			if (current_length < 0.05 * proj.length) {
				res.casing_failure = true;
				res.regime = "Hypervelocity Erosion Burnout";
				res.outcome_summary =
					"Projectile fully eroded by hydrodynamic penetration.";
				break;
			}
		}

		if (current_depth >= next_print_depth) {
			double g_force = acceleration / cons.gravity;
			std::cout << "  [Penetration T+ " << std::fixed << std::setprecision(2)
				  << (t * 1000.0) << " ms] Depth: " << std::setprecision(1)
				  << current_depth << " m | Vel: " << std::setprecision(1)
				  << current_velocity << " m/s | Decel: " << std::setprecision(0)
				  << g_force << " G | Temp: " << current_temperature
				  << " K | Layer: " << layer.material_name
				  << (erosion_active ? " | [ERODING]" : "") << "\n";
			next_print_depth += 1.0;
		}

		if (pen_frame_counter++ % 20 == 0) {
			TelemetryFrame frame;
			frame.time = t;
			frame.altitude = 0.0;
			frame.depth = current_depth;
			frame.velocity = current_velocity;
			frame.mach = current_velocity / groundSpeedOfSound;
			frame.dynamic_pressure = dynamic_pressure;
			frame.g_force = std::abs(acceleration / cons.gravity);
			frame.heat = std::min(1.0, current_temperature / proj.melting_point);
			frame.is_eroding = erosion_active;
			frame.dif = res.dynamic_increase_factor;
			frame.remaining_length = erosion_active ? current_length : proj.length;
			frame.obliquity_deg = obliquity_radians * 180.0 / cons.PI;
			res.penetration_frames.push_back(frame);
		}

		t += dt;

		if (t > 10.0)
			break;
	}

	if (current_velocity <= 0.0) {
		std::string final_layer = current_layer_idx < target.layers.size()
						  ? target.layers[current_layer_idx].material_name
						  : "Unknown";
		std::cout << "  [FULL STOP at T+ " << std::fixed << std::setprecision(2)
			  << (t * 1000.0)
			  << " ms] Projectile came to rest at Depth: " << std::setprecision(2)
			  << current_depth << " m inside layer: " << final_layer << "\n";
	}
	std::cout << "------------------------------------\n\n";

	target.pulverizeDepth(current_depth);
	res.cumulative_breach_depth = current_depth;

	res.actual_penetration_depth = current_depth;
	res.dynamic_pressure = max_dynamic_pressure;
	res.rigid_penetration = current_depth;

	double total_thickness = 0.0;
	double weighted_density_sum = 0.0;
	for (const auto& layer : target.layers) {
		weighted_density_sum += layer.density * layer.thickness;
		total_thickness += layer.thickness;
	}
	double default_density = target.layers.empty() ? 2500.0 : target.layers[0].density;
	double average_density =
		(total_thickness > 0) ? (weighted_density_sum / total_thickness) : default_density;
	res.hydro_penetration = proj.length * std::sqrt(proj.casing_density / average_density);

	res.is_kinetic_rod = (proj.explosive_mass == 0.0);
	if (res.is_kinetic_rod) {
		if (!res.casing_failure) {
			if (res.erosion_occurred) {
				res.regime = "Hypervelocity Erosion (Walker-Anderson)";
				res.outcome_summary =
					"Projectile eroded hydrodynamically; casing survived intact.";
			} else {
				res.regime = "Rigid Penetration (Crater+Tunnel)";
			}
		}
		res.shock_damage_prob_percent = 0.0;
		if (res.casing_failure) {
			res.explosive_charge_survives = false;
			res.premature_detonation = true;
		} else {
			res.explosive_charge_survives = true;
			if (res.erosion_occurred) {
				res.regime = "Hypervelocity Erosion (Walker-Anderson)";
			}
		}
	}

	res.explosive_mass = proj.explosive_mass;
	if (res.is_kinetic_rod) {
		res.explosion_scale = 1.0;
		res.crater_wide_radius = proj.diameter * 2.0;
	} else {
		res.explosion_scale = std::max(5.0, std::min(50.0, proj.explosive_mass / 50.0));
		res.crater_wide_radius = std::min(20.0, std::max(4.5, proj.explosive_mass / 100.0));
	}
	res.crater_narrow_radius = proj.diameter / 2.0;
	res.camera_shake_magnitude = std::min(1.5, res.kinetic_energy / 1e9);

	constexpr double desiredWallClockSeconds = 6.0;
	double totalPenSimTime =
		res.penetration_frames.empty() ? dt : res.penetration_frames.back().time;
	res.time_scale_pen =
		(totalPenSimTime > 1.0e-9)
			? std::clamp(desiredWallClockSeconds / totalPenSimTime, 0.01, 5000.0)
			: 0.02;

	res.x_acceleration = 0.0;
	res.y_acceleration = 0.0;
}

SimulationResult ImpactSimulator::simulate(const ImpactScenario& scenario) {



	SimulationResult res;
	res.scenario_name = scenario.name;
	res.altitude_ft = scenario.altitude_ft;
	res.velocity = scenario.velocity;

	res.mach_number = scenario.velocity / EnvironmentPhysics::standardAtmosphere(
						      scenario.altitude_ft / 3.28084, cons)
						      .speed_of_sound_ms;
	res.casing_failure = false;
	res.premature_detonation = false;
	res.explosive_charge_survives = true;
	res.shock_damage_prob_percent = 0.0;
	res.regime = "Rigid Penetration (Crater+Tunnel)";
	res.outcome_summary = "Intact";

	double impact_velocity = 0.0;
	double impact_pitch = 0.0;

	double dt = 1e-5;

	simulateAtmosphericDrop(scenario, proj, res, impact_velocity, impact_pitch, dt);

	simulateGroundPenetration(scenario, res, impact_velocity, impact_pitch, dt);

	res.kinetic_shock_joules = impactShockwave(proj.total_mass, impact_velocity);
	res.total_explosive_yield = explosiveShockwave(proj.explosive_mass, proj.explosive_energy_j_per_kg);

	return res;
}
