// Fill out your copyright notice in the Description page of Project Settings.


#include "Target.h"

// Sets default values
ATarget::ATarget() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// assign a mesh
	TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetMesh"));
	RootComponent = TargetMesh; // Make the mesh the root of the entire Actor!

	// Set Default Target Data (Fordow-like Hardened Mountain Facility)
	TargetPhysicsData.Name = TEXT("Fordow-like Hardened Mountain Facility");

	// Layer 1: Soil/Overburden
	FTargetLayer SoilLayer;
	SoilLayer.MaterialName = TEXT("Soil/Overburden + Surface Cap");
	SoilLayer.Thickness = 3.0;
	SoilLayer.Density = 1800.0;
	SoilLayer.CompressiveStrength = 20.0e6;
	SoilLayer.HugoniotC0 = 1800.0;
	SoilLayer.HugoniotS = 1.5;
	TargetPhysicsData.Layers.Add(SoilLayer);

	// Layer 2: Reinforced Concrete
	FTargetLayer ConcreteLayer;
	ConcreteLayer.MaterialName =
		TEXT("Reinforced Concrete (facility structure / shaft lining)");
	ConcreteLayer.Thickness = 5.0;
	ConcreteLayer.Density = 2650.0;
	ConcreteLayer.CompressiveStrength = 70.0e6;
	ConcreteLayer.RebarVolumeFraction = 0.04;
	ConcreteLayer.RebarYieldStrength = 500.0e6;
	ConcreteLayer.HugoniotC0 = 3200.0;
	ConcreteLayer.HugoniotS = 1.9;
	TargetPhysicsData.Layers.Add(ConcreteLayer);

	// Layer 3: Solid Mountain Rock
	FTargetLayer RockLayer;
	RockLayer.MaterialName = TEXT("Solid Mountain Rock (limestone/granite-like)");
	RockLayer.Thickness = 90.0;
	RockLayer.Density = 2750.0;
	RockLayer.CompressiveStrength = 170.0e6;
	RockLayer.HugoniotC0 = 3800.0;
	RockLayer.HugoniotS = 1.35;
	TargetPhysicsData.Layers.Add(RockLayer);

	// Trigger after impact
	TargetMesh->OnComponentHit.AddDynamic(this, &ATarget::OnTargetHit);
}

// on Impact Event
void ATarget::OnTargetHit(UPrimitiveComponent* HitComponent,
			  AActor* OtherActor,
			  UPrimitiveComponent* OtherComp,
			  FVector NormalImpulse,
			  const FHitResult& Hit) {
	// 1. Check if the thing that hit us is actually the Projectile!
	AProjectile* HittingBomb = Cast<AProjectile>(OtherActor);

	if (HittingBomb != nullptr) {

		// Receive bomb's data
		FProjectile BombData = HittingBomb->GetPhysicsData();


		// ! ********************
		// ! Shock Initiation (Walker-Wasley)
		// ! ********************
		ShockWaveIgnitionResult ImpactSimulator::shockWaveIgnition(double currentVelocity,
									   double impactVelocity,
									   double rhoT,
									   double pShock,
									   double TAU) {

			ShockWaveIgnitionResult shock;

			// Set initial dynamic pressure on impact so it is never 0.00 GPa
			shock.dynamic_pressure = 0.5 * rhoT * std::pow(currentVelocity, 2);

			double shock_transmission_coef = 0.25;
			double transmitted_pressure = pShock * shock_transmission_coef;

			shock.transmitted_pressure = transmitted_pressure;
			shock.shock_pressure_gpa_peak = transmitted_pressure / 1.0e9;
			shock.shock_pulse_duration_us = TAU * 1.0e6;

			double shock_energy = std::pow(transmitted_pressure, 2) * TAU;
			shock.shock_energy = shock_energy;

			shock.velocity = impactVelocity;
			shock.kinetic_energy = 0.5 * proj.total_mass * std::pow(impactVelocity, 2);

			if (proj.explosive_mass > 0.0 && proj.explosive_critical_energy > 0.0) {
				if (shock_energy >= proj.explosive_critical_energy) {
					shock.casing_failure = true;
					shock.explosive_charge_survives = false;
					shock.premature_detonation = true;
					shock.shock_damage_prob_percent = 100.0;
					shock.regime = "Shock Initiation (Walker-Wasley)";
					shock.outcome_summary =
						"Premature detonation triggered by Hugoniot impact shock.";
					shock.actual_penetration_depth = 0.0;
				}
			}

			return shock;
		}

		PostPenetrationCraterProfilingResult
		ImpactSimulator::postPenetrationCraterProfiling(double currentDepth,
								double maxDynamicPressure,
								bool casingFailure,
								bool erosionOccurred,
								double kineticEnergy) {

			PostPenetrationCraterProfilingResult PPCPR;

			target.pulverizeDepth(currentDepth);
			PPCPR.cumulative_breach_depth = currentDepth;
			PPCPR.actual_penetration_depth = currentDepth;
			PPCPR.rigid_penetration = currentDepth;

			PPCPR.dynamic_pressure = maxDynamicPressure;

			double total_thickness = 0.0;
			double weighted_density_sum = 0.0;

			for (const auto& layer : TargetPhysicsData.Layers) {
				weighted_density_sum += layer.density * layer.thickness;
				total_thickness += layer.thickness;
			}

			double default_density = TargetPhysicsData.Layers.empty()
							 ? 2500.0
							 : TargetPhysicsData.Layers[0].density;

			double average_density = (total_thickness > 0)
							 ? (weighted_density_sum / total_thickness)
							 : default_density;

			PPCPR.hydro_penetration =
				proj.length * std::sqrt(proj.casing_density / average_density);

			PPCPR.is_kinetic_rod = (proj.explosive_mass == 0.0);

			if (!casingFailure) {
				if (erosionOccurred) {
					PPCPR.regime = "Hypervelocity Erosion (Walker-Anderson)";
					PPCPR.outcome_summary =
						"Projectile eroded hydrodynamically; casing survived intact.";
				} else {
					PPCPR.regime = "Rigid Penetration (Crater+Tunnel)";
				}
			}

			if (PPCPR.is_kinetic_rod) {
				PPCPR.shock_damage_prob_percent = 0.0;
			}

			if (casingFailure) {
				PPCPR.explosive_charge_survives = false;
				PPCPR.premature_detonation = true;
			} else {
				PPCPR.explosive_charge_survives = true;
			}

			PPCPR.explosive_mass = proj.explosive_mass;
			if (PPCPR.is_kinetic_rod) {
				PPCPR.explosion_scale = 1.0;
				PPCPR.crater_wide_radius = proj.diameter * 2.0;
			} else {
				PPCPR.explosion_scale =
					std::max(5.0, std::min(50.0, proj.explosive_mass / 50.0));
				PPCPR.crater_wide_radius =
					std::min(20.0, std::max(4.5, proj.explosive_mass / 100.0));
			}
			PPCPR.crater_narrow_radius = proj.diameter / 2.0;
			PPCPR.camera_shake_magnitude = std::min(1.5, kineticEnergy / 1e9);

			return PPCPR;
		}


		ThermalMassAblationResult ImpactSimulator::thermalMassAblation(
			bool erosionActive,
			double& currentTemperature,
			double& currentMass,
			double& currentLength) {
			ThermalMassAblationResult TMA;

			if (!erosionActive) {
				if (currentTemperature > proj.melting_point) {
					double excess_temp =
						currentTemperature - proj.melting_point;
					double excess_heat =
						excess_temp * currentMass * proj.specific_heat;

					if (excess_heat > 0 && proj.heat_of_fusion > 0) {
						double mass_loss =
							excess_heat / proj.heat_of_fusion;
						currentMass -= mass_loss;
						currentTemperature = proj.melting_point;

						if (currentMass < 0.1 * proj.total_mass) {
							TMA.casing_failure = true;
							TMA.regime = "Thermal Destruction";
							TMA.outcome_summary =
								"Projectile completely ablated.";
							TMA.should_break = true;
						}
					}
				}
			} else {
				currentLength = std::max(0.0, currentLength);
				double effective_linear_density = proj.total_mass / proj.length;
				currentMass = effective_linear_density * currentLength;
				TMA.final_rod_length = currentLength;
				TMA.erosion_length_lost = proj.length - currentLength;

				if (currentLength < 0.05 * proj.length) {
					TMA.casing_failure = true;
					TMA.regime = "Hypervelocity Erosion Burnout";
					TMA.outcome_summary =
						"Projectile fully eroded by hydrodynamic penetration.";
					TMA.should_break = true;
				}
			}

			return TMA;
		}

		TelemetryFrame ImpactSimulator::buildPenetrationFrame(const FramePackPayload& p) {
			TelemetryFrame frame;
			frame.time = p.t;
			frame.altitude = 0.0;
			frame.depth = p.current_depth;
			frame.velocity = p.current_velocity;
			frame.mach = p.current_velocity / p.groundSpeedOfSound;
			frame.dynamic_pressure = p.dynamic_pressure;
			frame.g_force = std::abs(p.acceleration / cons.gravity);
			frame.heat = std::min(1.0, p.current_temperature / proj.melting_point);
			frame.is_eroding = p.erosion_active;
			frame.dif = p.dynamic_increase_factor;
			frame.remaining_length = p.erosion_active ? p.current_length : proj.length;
			frame.obliquity_deg = p.obliquity_radians * 180.0 / cons.PI;

			frame.current_vx = 0.0;
			frame.current_vy = p.current_velocity;
			frame.Up = p.Up;
			frame.Us = p.Us;
			frame.P_shock = p.P_shock;
			frame.transmitted_pressure = p.transmitted_pressure;
			frame.shock_energy = p.shock_energy;

			frame.asymmetric_force = p.asymmetric_force;
			frame.bending_moment = p.bending_moment;
			frame.max_bending_stress = p.max_bending_stress;

			frame.strain_rate =
				std::fabs(p.current_velocity) / std::max(0.01, proj.diameter);
			frame.effective_strength = p.baseStrength * p.dynamic_increase_factor;

			double fc_mpa = std::max(0.001, frame.effective_strength / 1.0e6);
			double S = 82.6 * std::pow(fc_mpa, -0.544);
			double CRH_val = (proj.diameter > 0.0)
						 ? (proj.curvature_noseReduce / proj.diameter)
						 : 3.0;
			double dragCoef = (8.0 * ((CRH_val > 0.0) ? CRH_val : 3.0) - 1.0) /
					  (24.0 * std::pow(((CRH_val > 0.0) ? CRH_val : 3.0), 2));
			frame.tunnel_force = (cons.PI * std::pow(proj.diameter / 2.0, 2)) *
					     (S * frame.effective_strength +
					      dragCoef * p.baseDensity * p.current_velocity *
						      p.current_velocity);

			frame.interface_erosion_velocity =
				p.erosion_active ? (p.current_velocity *
						    std::sqrt(p.baseDensity /
							      std::max(1.0, proj.casing_density)))
						 : 0.0;

			frame.heat_rate = (p.current_temperature > proj.melting_point)
						  ? (p.current_temperature - proj.melting_point)
						  : 0.0;
			frame.excess_heat =
				(p.current_temperature > proj.melting_point)
					? ((p.current_temperature - proj.melting_point) *
					   p.current_mass * proj.specific_heat)
					: 0.0;
			frame.mass_loss = (proj.heat_of_fusion > 0 && frame.excess_heat > 0)
						  ? (frame.excess_heat / proj.heat_of_fusion)
						  : 0.0;
			frame.effective_linear_density =
				p.erosion_active
					? (proj.total_mass / proj.length)
					: (p.current_mass / std::max(0.01, p.current_length));

			return frame;
		}

		// ! ********************
		// ! Penetration in ground
		// ! ********************
		void ImpactSimulator::simulateGroundPenetration(const ImpactScenario& scenario,
								SimulationResult& res,
								double impact_velocity,
								double impact_pitch,
								double dt) {


			double target_obliquity_radians =
				scenario.obliquity_angle * cons.PI / 180.0;
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
			double bar_wave_speed =
				std::sqrt(proj.elastic_modulus / proj.casing_density);
			res.bar_wave_speed = bar_wave_speed;

			double initial_shaft_depth = 0.0;
			for (const auto& layer : TargetPhysicsData.Layers) {
				if (layer.pulverized_depth > 0) {
					initial_shaft_depth +=
						std::min(layer.thickness, layer.pulverized_depth);
				}
			}
			res.previous_strike_depth = initial_shaft_depth;
			if (initial_shaft_depth > 0) {
				current_depth = initial_shaft_depth;
				std::cout
					<< "  [SEQUENTIAL SALVO STRIKE] Entering pre-existing breached shaft depth: "
					<< initial_shaft_depth << " m\n";
			}

			std::vector<double> layer_bottom_depths_local;
			double fullDepth = 0.0;
			for (const auto& layer : TargetPhysicsData.Layers) {
				fullDepth += layer.thickness;
				layer_bottom_depths_local.push_back(fullDepth);
			}
			res.layer_bottom_depths = layer_bottom_depths_local;

			size_t current_layer_idx = 0;
			while (current_layer_idx < layer_bottom_depths_local.size() &&
			       current_depth >= layer_bottom_depths_local[current_layer_idx]) {
				current_layer_idx++;
			}
			size_t last_layer_idx = current_layer_idx;
			double next_print_depth = std::floor(current_depth) + 1.0;
			int pen_frame_counter = 0;

			if (current_layer_idx < TargetPhysicsData.Layers.size()) {
				std::cout << "--- Ground Penetration Commenced ---\n";
				std::cout
					<< "  [LAYER BREACH] Entering layer: "
					<< TargetPhysicsData.Layers[current_layer_idx].material_name
					<< "\n";
			}

			double critical_angle_threshold = 65.0 * cons.PI / 180.0;
			if (current_velocity < 200.0) {
				critical_angle_threshold = 50.0 * cons.PI / 180.0;
			}
			res.critical_angle_threshold = critical_angle_threshold;

			if (std::abs(obliquity_radians + angleOfAttack_radians) >=
			    critical_angle_threshold) {
				res.casing_failure = true;
				res.regime = "Ricochet";
				res.outcome_summary = "Projectile deflected off target surface.";
				res.actual_penetration_depth = 0.0;
				return;
			}



			const double groundSpeedOfSound =
				EnvironmentPhysics::standardAtmosphere(0.0, cons).speed_of_sound_ms;
			res.mach_number = current_velocity / groundSpeedOfSound;

			double rho_t = TargetPhysicsData.Layers.empty()
					       ? 2500.0
					       : TargetPhysicsData.Layers[0].density;
			double rho_p = proj.casing_density > 0 ? proj.casing_density : 7800.0;
			double Up = current_velocity / (1.0 + std::sqrt(rho_t / rho_p));
			double c0 = proj.hugoniot_c0 > 0 ? proj.hugoniot_c0 : 4570.0;
			double s_coef = proj.hugoniot_s > 0 ? proj.hugoniot_s : 1.49;
			double Us = c0 + s_coef * Up;
			double P_shock = rho_p * Us * Up;
			double wall_thick =
				proj.casing_wall_thickness > 0 ? proj.casing_wall_thickness : 0.05;
			double tau = (2.0 * wall_thick) / c0;

			ShockWaveIgnitionResult walkerWasley = shockWaveIgnition(
				current_velocity, impact_velocity, rho_t, P_shock, tau);

			max_dynamic_pressure = walkerWasley.dynamic_pressure;
			res.dynamic_pressure = max_dynamic_pressure;
			res.shock_pressure_gpa_peak = walkerWasley.shock_pressure_gpa_peak;
			res.shock_pulse_duration_us = walkerWasley.shock_pulse_duration_us;
			res.velocity = walkerWasley.velocity;
			res.kinetic_energy = walkerWasley.kinetic_energy;

			double transmitted_pressure = walkerWasley.transmitted_pressure;
			double shock_energy = walkerWasley.shock_energy;

			if (walkerWasley.premature_detonation) {
				res.casing_failure = walkerWasley.casing_failure;
				res.explosive_charge_survives =
					walkerWasley.explosive_charge_survives;
				res.premature_detonation = walkerWasley.premature_detonation;
				res.shock_damage_prob_percent =
					walkerWasley.shock_damage_prob_percent;
				res.regime = walkerWasley.regime;
				res.outcome_summary = walkerWasley.outcome_summary;
				res.actual_penetration_depth =
					walkerWasley.actual_penetration_depth;
				return;
			}


			double t = 0.0;

			struct PenDeriv {
				double dv = 0.0;
				double dz = 0.0;
				double dtheta = 0.0;
				double dT = 0.0;
				double dL = 0.0;
			};

			while (current_velocity > 0.0 && !res.casing_failure &&
			       current_depth < fullDepth) {
				while (current_layer_idx < layer_bottom_depths_local.size() &&
				       current_depth >=
					       layer_bottom_depths_local[current_layer_idx]) {
					current_layer_idx++;
				}

				if (current_layer_idx >= TargetPhysicsData.Layers.size()) {
					res.regime = "Target Perforated";
					res.outcome_summary =
						"Projectile completely pierced all target layers.";
					break;
				}

				if (current_layer_idx != last_layer_idx) {
					last_layer_idx = current_layer_idx;
					std::cout << "  [LAYER BREACH] Pierced into layer: "
						  << TargetPhysicsData.Layers[current_layer_idx]
							     .material_name
						  << "\n";
				}

				const auto& layer = TargetPhysicsData.Layers[current_layer_idx];
				double layerEntryDepth =
					(current_layer_idx == 0)
						? 0.0
						: layer_bottom_depths_local[current_layer_idx - 1];

				double squaredVelocity = current_velocity * current_velocity;
				double baseStrength =
					layer.compressive_strength +
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
						<< current_depth
						<< " m | Velocity: " << current_velocity
						<< " m/s\n";
				}

				double asymmetric_force = 0.0;
				double bending_moment = 0.0;
				double max_bending_stress = 0.0;
				if (std::abs(obliquity_radians) > 0.0 ||
				    std::abs(angleOfAttack_radians) > 0.0) {
					asymmetric_force =
						(0.5 * baseDensity * squaredVelocity * area) *
						std::sin(obliquity_radians + angleOfAttack_radians);
					bending_moment =
						std::abs(asymmetric_force) * (proj.length / 2.0);

					if (proj.area_moment_inertia > 0) {
						max_bending_stress =
							(bending_moment * (proj.diameter / 2.0)) /
							proj.area_moment_inertia;
					}

					if (proj.yield_strength > 0.0 &&
					    max_bending_stress > proj.yield_strength) {
						res.casing_failure = true;
						res.regime = "Structural Failure (J-Hook/Snap)";
						res.outcome_summary =
							"Bending moments exceeded casing yield strength.";
						break;
					}
				}

				auto derivative = [&](double v,
						      double z,
						      double theta,
						      double T,
						      double L,
						      double m) -> PenDeriv {
					(void)T; // Suppress unused parameter warning
					PenDeriv d;
					double vSq = v * v;
					double strain_rate =
						std::fabs(v) / std::max(0.01, proj.diameter);
					double dif = EnvironmentPhysics::computeDIF(strain_rate,
										    baseStrength);
					res.dynamic_increase_factor = dif;
					double effective_strength = baseStrength * dif;

					double lateral_force = 0.0;
					if (std::abs(theta) > 0.0 ||
					    std::abs(angleOfAttack_radians) > 0.0) {
						lateral_force =
							(0.5 * baseDensity * vSq * area) *
							std::sin(theta + angleOfAttack_radians);
					}
					double safeMass = std::max(0.001, m);
					double gravity_component = cons.gravity * std::cos(theta);

					if (!erosion_active) {
						double fc_mpa =
							std::max(0.001, effective_strength / 1.0e6);
						double S = 82.6 * std::pow(fc_mpa, -0.544);

						double CRH_val =
							(proj.diameter > 0.0)
								? (proj.curvature_noseReduce /
								   proj.diameter)
								: 3.0;
						double dragCoef =
							(8.0 * ((CRH_val > 0.0) ? CRH_val : 3.0) -
							 1.0) /
							(24.0 *
							 std::pow(((CRH_val > 0.0) ? CRH_val : 3.0),
								  2));

						double tunnelForce =
							area * (S * effective_strength +
								dragCoef * baseDensity * vSq);
						double craterDepthLimit = 2.0 * proj.diameter;
						double zLocal = z - layerEntryDepth;
						double axialForce =
							(craterDepthLimit > 0.0 &&
							 zLocal < craterDepthLimit)
								? tunnelForce *
									  std::clamp(
										  zLocal /
											  craterDepthLimit,
										  0.0,
										  1.0)
								: tunnelForce;

						d.dv = gravity_component - (axialForce / safeMass);
						d.dz = v * std::cos(theta);
						d.dL = 0.0;

						double heat_rate =
							(axialForce * cons.frictionFactor) *
							std::fabs(v);
						d.dT = heat_rate / (safeMass * proj.specific_heat);
					} else {
						double u =
							EnvironmentPhysics::solveInterfaceVelocity(
								v,
								proj.casing_density,
								baseDensity,
								proj.yield_strength,
								effective_strength);
						double Le = std::max(0.01, L);
						d.dv = -(proj.yield_strength /
							 (proj.casing_density * Le)) *
							       (1.0 + (v - u) / bar_wave_speed) +
						       gravity_component;
						d.dz = u * std::cos(theta);
						d.dL = -(v - u);

						double erosion_heat_rate = 0.5 * baseDensity *
									   (v - u) * (v - u) *
									   area * std::fabs(v - u);
						d.dT = erosion_heat_rate /
						       (safeMass * proj.specific_heat);
					}

					if (v > 0.1) {
						d.dtheta = (lateral_force / safeMass) / v;
					}

					return d;
				};

				auto get_mass = [&](double L_eval) {
					return erosion_active ? (proj.total_mass / proj.length) *
									std::max(0.0, L_eval)
							      : current_mass;
				};

				PenDeriv k1 = derivative(current_velocity,
							 current_depth,
							 obliquity_radians,
							 current_temperature,
							 current_length,
							 get_mass(current_length));
				PenDeriv k2 =
					derivative(current_velocity + 0.5 * dt * k1.dv,
						   current_depth + 0.5 * dt * k1.dz,
						   obliquity_radians + 0.5 * dt * k1.dtheta,
						   current_temperature + 0.5 * dt * k1.dT,
						   current_length + 0.5 * dt * k1.dL,
						   get_mass(current_length + 0.5 * dt * k1.dL));
				PenDeriv k3 =
					derivative(current_velocity + 0.5 * dt * k2.dv,
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

				current_velocity +=
					(dt / 6.0) * (k1.dv + 2 * k2.dv + 2 * k3.dv + k4.dv);
				current_depth +=
					(dt / 6.0) * (k1.dz + 2 * k2.dz + 2 * k3.dz + k4.dz);
				obliquity_radians += (dt / 6.0) * (k1.dtheta + 2 * k2.dtheta +
								   2 * k3.dtheta + k4.dtheta);
				current_temperature +=
					(dt / 6.0) * (k1.dT + 2 * k2.dT + 2 * k3.dT + k4.dT);
				current_length +=
					(dt / 6.0) * (k1.dL + 2 * k2.dL + 2 * k3.dL + k4.dL);

				// !
				// !
				// !

				ThermalMassAblationResult TMAR =
					thermalMassAblation(erosion_active,
							    current_temperature,
							    current_mass,
							    current_length);

				if (TMAR.casing_failure) {
					res.casing_failure = TMAR.casing_failure;
					res.regime = TMAR.regime;
					res.outcome_summary = TMAR.outcome_summary;
				}
				if (erosion_active) {
					res.final_rod_length = TMAR.final_rod_length;
					res.erosion_length_lost = TMAR.erosion_length_lost;
				}

				if (TMAR.should_break) {
					break;
				}

				// !
				// !
				// !

				if (current_depth >= next_print_depth) {
					double g_force = acceleration / cons.gravity;
					std::cout << "  [Penetration T+ " << std::fixed
						  << std::setprecision(2) << (t * 1000.0)
						  << " ms] Depth: " << std::setprecision(1)
						  << current_depth
						  << " m | Vel: " << std::setprecision(1)
						  << current_velocity
						  << " m/s | Decel: " << std::setprecision(0)
						  << g_force << " G | Temp: " << current_temperature
						  << " K | Layer: " << layer.material_name
						  << (erosion_active ? " | [ERODING]" : "") << "\n";
					next_print_depth += 1.0;
				}

				if (pen_frame_counter++ % 20 == 0) {
					FramePackPayload payload = {t,
								    current_depth,
								    current_velocity,
								    groundSpeedOfSound,
								    dynamic_pressure,
								    acceleration,
								    current_temperature,
								    erosion_active,
								    res.dynamic_increase_factor,
								    current_length,
								    obliquity_radians,
								    Up,
								    Us,
								    P_shock,
								    transmitted_pressure,
								    shock_energy,
								    asymmetric_force,
								    bending_moment,
								    max_bending_stress,
								    baseStrength,
								    baseDensity,
								    current_mass};
					TelemetryFrame frame = buildPenetrationFrame(payload);

					res.penetration_frames.push_back(frame);
				}

				t += dt;

				if (t > 10.0)
					break;
			}

			if (current_velocity <= 0.0) {
				std::string final_layer =
					current_layer_idx < TargetPhysicsData.Layers.size()
						? TargetPhysicsData.Layers[current_layer_idx]
							  .material_name
						: "Unknown";
				std::cout << "  [FULL STOP at T+ " << std::fixed
					  << std::setprecision(2) << (t * 1000.0)
					  << " ms] Projectile came to rest at Depth: "
					  << std::setprecision(2) << current_depth
					  << " m inside layer: " << final_layer << "\n";
			}
			std::cout << "------------------------------------\n\n";

			PostPenetrationCraterProfilingResult PPCP =
				postPenetrationCraterProfiling(current_depth,
							       max_dynamic_pressure,
							       res.casing_failure,
							       res.erosion_occurred,
							       res.kinetic_energy);

			res.cumulative_breach_depth = PPCP.cumulative_breach_depth;
			res.actual_penetration_depth = PPCP.actual_penetration_depth;
			res.rigid_penetration = PPCP.rigid_penetration;
			res.dynamic_pressure = PPCP.dynamic_pressure;
			res.hydro_penetration = PPCP.hydro_penetration;
			res.is_kinetic_rod = PPCP.is_kinetic_rod;
			if (!PPCP.regime.empty()) {
				res.regime = PPCP.regime;
			}
			if (!PPCP.outcome_summary.empty()) {
				res.outcome_summary = PPCP.outcome_summary;
			}
			res.shock_damage_prob_percent = PPCP.shock_damage_prob_percent;
			res.explosive_charge_survives = PPCP.explosive_charge_survives;
			res.premature_detonation = PPCP.premature_detonation;
			res.explosive_mass = PPCP.explosive_mass;
			res.explosion_scale = PPCP.explosion_scale;
			res.crater_wide_radius = PPCP.crater_wide_radius;
			res.crater_narrow_radius = PPCP.crater_narrow_radius;
			res.camera_shake_magnitude = PPCP.camera_shake_magnitude;
			// !
			// !

			constexpr double desiredWallClockSeconds = 6.0;
			double totalPenSimTime = res.penetration_frames.empty()
							 ? dt
							 : res.penetration_frames.back().time;
			res.time_scale_pen =
				(totalPenSimTime > 1.0e-9)
					? std::clamp(desiredWallClockSeconds / totalPenSimTime,
						     0.01,
						     5000.0)
					: 0.02;

			res.x_acceleration = 0.0;
			res.y_acceleration = 0.0;
		}

		// IT IS THE BOMB!
		// 2. THIS is where you paste your MASSIVE while-loop from `simulation.cpp`!
		// You will calculate the penetration depth using `TargetPhysicsData.Layers`
		// and the bomb's mass!
	}
}


// Called when the game starts or when spawned
void ATarget::BeginPlay() {
	Super::BeginPlay();
}

// Called every frame
void ATarget::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}
