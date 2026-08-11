// Fill out your copyright notice in the Description page of Project Settings.


#include "Target.h"
#include "Projectile.h"

namespace
{
	struct FPhysicsConstants {
		double gravity = 9.81;
		double PI = 3.14159265358979323846;
		double frictionFactor = 0.1;
		double universalGasConstant = 8.31432;
		double molarMassAir = 0.0289644;
		double adiabaticIndexAir = 1.4;
		double earthRadius = 6356766.0;
	};

	struct FAtmosphereState {
		double temperature_K = 288.15;
		double pressure_Pa = 101325.0;
		double density_kgm3 = 1.225;
		double speed_of_sound_ms = 340.3;
	};

	struct FShockWaveIgnitionResult {
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
		FString regime = TEXT("");
		FString outcome_summary = TEXT("");
	};

	struct FPostPenetrationCraterProfilingResult {
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
		FString regime = TEXT("");
		FString outcome_summary = TEXT("");
	};

	struct FThermalMassAblationResult {
		double final_rod_length = 0.0;
		double erosion_length_lost = 0.0;
		bool casing_failure = false;
		bool should_break = false;
		FString regime = TEXT("");
		FString outcome_summary = TEXT("");
	};

	struct FPenDeriv {
		double dv = 0.0;
		double dz = 0.0;
		double dtheta = 0.0;
		double dT = 0.0;
		double dL = 0.0;
	};

	// Helper Math Functions
	FAtmosphereState StandardAtmosphere(double altitude_m, const FPhysicsConstants& cons) {
		struct AtmosphereLayer {
			double base_geopotential_m;
			double base_temperature_K;
			double base_pressure_Pa;
			double lapse_rate_Kpm;
		};
		static constexpr AtmosphereLayer layers[] = {
			{0.0, 288.15, 101325.0, -0.0065},
			{11000.0, 216.65, 22632.1, 0.0},
			{20000.0, 216.65, 5474.89, 0.0010},
			{32000.0, 228.65, 868.019, 0.0028},
			{47000.0, 270.65, 110.906, 0.0},
			{51000.0, 270.65, 66.9389, -0.0028},
			{71000.0, 214.65, 3.95642, -0.0020},
		};
		constexpr size_t layerCount = sizeof(layers) / sizeof(layers[0]);
		constexpr double topGeopotential_m = 84852.0;

		double h = (cons.earthRadius * altitude_m) / (cons.earthRadius + altitude_m);
		double h_eval = FMath::Min(h, topGeopotential_m);

		size_t idx = 0;
		for (size_t i = 0; i < layerCount; ++i) {
			if (h_eval >= layers[i].base_geopotential_m) {
				idx = i;
			}
		}
		const AtmosphereLayer& base = layers[idx];

		double g0M_over_R = (cons.gravity * cons.molarMassAir) / cons.universalGasConstant;
		double T, P;
		if (FMath::Abs(base.lapse_rate_Kpm) > 1e-12) {
			T = base.base_temperature_K + base.lapse_rate_Kpm * (h_eval - base.base_geopotential_m);
			P = base.base_pressure_Pa * FMath::Pow(base.base_temperature_K / T, g0M_over_R / base.lapse_rate_Kpm);
		} else {
			T = base.base_temperature_K;
			P = base.base_pressure_Pa * FMath::Exp(-g0M_over_R * (h_eval - base.base_geopotential_m) / T);
		}
		if (h > topGeopotential_m) {
			P *= FMath::Exp(-g0M_over_R * (h - topGeopotential_m) / T);
		}

		FAtmosphereState state;
		state.temperature_K = T;
		state.pressure_Pa = P;
		state.density_kgm3 = (P * cons.molarMassAir) / (cons.universalGasConstant * T);
		state.speed_of_sound_ms = FMath::Sqrt((cons.adiabaticIndexAir * cons.universalGasConstant * T) / cons.molarMassAir);
		return state;
	}

	double ComputeDIF(double strain_rate_per_s, double fc_static_pa) {
		constexpr double staticReferenceStrainRate = 30.0e-6; 
		constexpr double referenceStress_fco = 10.0e6;	      
		double fc_static = FMath::Max(1.0e5, fc_static_pa);
		double alpha = 1.0 / (5.0 + 9.0 * (fc_static / referenceStress_fco));
		double strain_rate = FMath::Max(strain_rate_per_s, staticReferenceStrainRate);

		if (strain_rate <= 30.0) {
			return FMath::Pow(strain_rate / staticReferenceStrainRate, 1.026 * alpha);
		}
		double logGamma = 6.156 * alpha - 2.0;
		double gamma = FMath::Pow(10.0, logGamma);
		return gamma * FMath::Pow(strain_rate / staticReferenceStrainRate, 1.0 / 3.0);
	}

	double SolveInterfaceVelocity(double v, double rho_p, double rho_t, double Yp, double Rt) {
		double A = 0.5 * (rho_p - rho_t);
		double B = -rho_p * v;
		double C = 0.5 * rho_p * v * v - (Rt - Yp);

		if (FMath::Abs(A) < 1.0e-6) {
			if (FMath::Abs(B) < 1.0e-9) return 0.0;
			return FMath::Clamp(-C / B, 0.0, v);
		}
		double discriminant = B * B - 4.0 * A * C;
		if (discriminant < 0.0) return 0.0; 

		double sqrtDisc = FMath::Sqrt(discriminant);
		double root1 = (-B + sqrtDisc) / (2.0 * A);
		double root2 = (-B - sqrtDisc) / (2.0 * A);

		bool root1Valid = (root1 >= 0.0 && root1 <= v);
		bool root2Valid = (root2 >= 0.0 && root2 <= v);

		if (root1Valid && root2Valid) return FMath::Min(root1, root2);
		if (root1Valid) return root1;
		if (root2Valid) return root2;
		return 0.0;
	}

	FShockWaveIgnitionResult ShockWaveIgnition(double currentVelocity, double impactVelocity, double rhoT, double pShock, double TAU, const FProjectile& proj) {
		FShockWaveIgnitionResult shock;
		shock.dynamic_pressure = 0.5 * rhoT * FMath::Pow(currentVelocity, 2);
		double shock_transmission_coef = 0.25;
		double transmitted_pressure = pShock * shock_transmission_coef;

		shock.transmitted_pressure = transmitted_pressure;
		shock.shock_pressure_gpa_peak = transmitted_pressure / 1.0e9;
		shock.shock_pulse_duration_us = TAU * 1.0e6;
		double shock_energy = FMath::Pow(transmitted_pressure, 2) * TAU;
		shock.shock_energy = shock_energy;
		shock.velocity = impactVelocity;
		shock.kinetic_energy = 0.5 * proj.TotalMass * FMath::Pow(impactVelocity, 2);

		if (proj.ExplosiveMass > 0.0 && proj.ExplosiveCriticalEnergy > 0.0) {
			if (shock_energy >= proj.ExplosiveCriticalEnergy) {
				shock.casing_failure = true;
				shock.explosive_charge_survives = false;
				shock.premature_detonation = true;
				shock.shock_damage_prob_percent = 100.0;
				shock.regime = TEXT("Shock Initiation (Walker-Wasley)");
				shock.outcome_summary = TEXT("Premature detonation triggered by Hugoniot impact shock.");
				shock.actual_penetration_depth = 0.0;
			}
		}
		return shock;
	}

	FPostPenetrationCraterProfilingResult PostPenetrationCraterProfiling(double currentDepth, double maxDynamicPressure, bool casingFailure, bool erosionOccurred, double kineticEnergy, const FProjectile& proj, const FTarget& TargetPhysicsData) {
		FPostPenetrationCraterProfilingResult PPCPR;
		PPCPR.cumulative_breach_depth = currentDepth;
		PPCPR.actual_penetration_depth = currentDepth;
		PPCPR.rigid_penetration = currentDepth;
		PPCPR.dynamic_pressure = maxDynamicPressure;

		double total_thickness = 0.0;
		double weighted_density_sum = 0.0;

		for (const auto& layer : TargetPhysicsData.Layers) {
			weighted_density_sum += layer.Density * layer.Thickness;
			total_thickness += layer.Thickness;
		}

		double default_density = TargetPhysicsData.Layers.Num() == 0 ? 2500.0 : TargetPhysicsData.Layers[0].Density;
		double average_density = (total_thickness > 0) ? (weighted_density_sum / total_thickness) : default_density;

		PPCPR.hydro_penetration = proj.Length * FMath::Sqrt(proj.CasingDensity / average_density);
		PPCPR.is_kinetic_rod = (proj.ExplosiveMass == 0.0);

		if (!casingFailure) {
			if (erosionOccurred) {
				PPCPR.regime = TEXT("Hypervelocity Erosion (Walker-Anderson)");
				PPCPR.outcome_summary = TEXT("Projectile eroded hydrodynamically; casing survived intact.");
			} else {
				PPCPR.regime = TEXT("Rigid Penetration (Crater+Tunnel)");
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

		PPCPR.explosive_mass = proj.ExplosiveMass;
		if (PPCPR.is_kinetic_rod) {
			PPCPR.explosion_scale = 1.0;
			PPCPR.crater_wide_radius = proj.Diameter * 2.0;
		} else {
			PPCPR.explosion_scale = FMath::Max(5.0, FMath::Min(50.0, proj.ExplosiveMass / 50.0));
			PPCPR.crater_wide_radius = FMath::Min(20.0, FMath::Max(4.5, proj.ExplosiveMass / 100.0));
		}
		PPCPR.crater_narrow_radius = proj.Diameter / 2.0;
		PPCPR.camera_shake_magnitude = FMath::Min(1.5, kineticEnergy / 1e9);

		return PPCPR;
	}

	FThermalMassAblationResult ThermalMassAblation(bool erosionActive, double& currentTemperature, double& currentMass, double& currentLength, const FProjectile& proj) {
		FThermalMassAblationResult TMA;
		if (!erosionActive) {
			if (currentTemperature > proj.MeltingPoint) {
				double excess_temp = currentTemperature - proj.MeltingPoint;
				double excess_heat = excess_temp * currentMass * proj.SpecificHeat;
				if (excess_heat > 0 && proj.HeatOfFusion > 0) {
					double mass_loss = excess_heat / proj.HeatOfFusion;
					currentMass -= mass_loss;
					currentTemperature = proj.MeltingPoint;
					if (currentMass < 0.1 * proj.TotalMass) {
						TMA.casing_failure = true;
						TMA.regime = TEXT("Thermal Destruction");
						TMA.outcome_summary = TEXT("Projectile completely ablated.");
						TMA.should_break = true;
					}
				}
			}
		} else {
			currentLength = FMath::Max(0.0, currentLength);
			double effective_linear_density = proj.TotalMass / proj.Length;
			currentMass = effective_linear_density * currentLength;
			TMA.final_rod_length = currentLength;
			TMA.erosion_length_lost = proj.Length - currentLength;
			if (currentLength < 0.05 * proj.Length) {
				TMA.casing_failure = true;
				TMA.regime = TEXT("Hypervelocity Erosion Burnout");
				TMA.outcome_summary = TEXT("Projectile fully eroded by hydrodynamic penetration.");
				TMA.should_break = true;
			}
		}
		return TMA;
	}
}

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
	ConcreteLayer.MaterialName = TEXT("Reinforced Concrete (facility structure / shaft lining)");
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
        
        // Disable bomb physics so it stops passing through while we calculate
        if (UPrimitiveComponent* BombComp = Cast<UPrimitiveComponent>(HittingBomb->GetRootComponent())) {
            BombComp->SetSimulatePhysics(false);
        }

		// Constants for calculation
		FPhysicsConstants cons;
		double impact_velocity = HittingBomb->GetVelocity().Size() / 100.0; // cm/s to m/s
        if (impact_velocity < 10.0) impact_velocity = 300.0; // Fallback if bomb had no velocity

		double impact_pitch = 0.0; 
		double dt = 0.0001;
        
		double current_velocity = impact_velocity;
		double current_depth = 0.0;
		double current_mass = BombData.TotalMass;
		double current_temperature = 300.0;
		double current_length = BombData.Length;
		double area = cons.PI * FMath::Pow(BombData.Diameter / 2.0, 2);

		double max_dynamic_pressure = 0.0;
		bool erosion_active = false;
		double bar_wave_speed = FMath::Sqrt(BombData.ElasticModulus / FMath::Max(1.0, BombData.CasingDensity));
        bool casing_failure = false;
        
		TArray<double> layer_bottom_depths_local;
		double fullDepth = 0.0;
		for (const auto& layer : TargetPhysicsData.Layers) {
			fullDepth += layer.Thickness;
			layer_bottom_depths_local.Add(fullDepth);
		}

		size_t current_layer_idx = 0;
		size_t last_layer_idx = current_layer_idx;
		double next_print_depth = FMath::Floor(current_depth) + 1.0;

		if (current_layer_idx < TargetPhysicsData.Layers.Num()) {
            UE_LOG(LogTemp, Warning, TEXT("--- Ground Penetration Commenced ---"));
			UE_LOG(LogTemp, Warning, TEXT("  [LAYER BREACH] Entering layer: %s"), *TargetPhysicsData.Layers[current_layer_idx].MaterialName);
		}

		double critical_angle_threshold = 65.0 * cons.PI / 180.0;
		if (current_velocity < 200.0) critical_angle_threshold = 50.0 * cons.PI / 180.0;

		const double groundSpeedOfSound = StandardAtmosphere(0.0, cons).speed_of_sound_ms;
		double rho_t = TargetPhysicsData.Layers.Num() == 0 ? 2500.0 : TargetPhysicsData.Layers[0].Density;
		double rho_p = BombData.CasingDensity > 0 ? BombData.CasingDensity : 7800.0;
		double Up = current_velocity / (1.0 + FMath::Sqrt(rho_t / rho_p));
		double c0 = BombData.HugoniotC0 > 0 ? BombData.HugoniotC0 : 4570.0;
		double s_coef = BombData.HugoniotS > 0 ? BombData.HugoniotS : 1.49;
		double Us = c0 + s_coef * Up;
		double P_shock = rho_p * Us * Up;
		double wall_thick = BombData.CasingWallThickness > 0 ? BombData.CasingWallThickness : 0.05;
		double tau = (2.0 * wall_thick) / c0;

		FShockWaveIgnitionResult walkerWasley = ShockWaveIgnition(current_velocity, impact_velocity, rho_t, P_shock, tau, BombData);
		max_dynamic_pressure = walkerWasley.dynamic_pressure;

		if (walkerWasley.premature_detonation) {
            casing_failure = true;
			UE_LOG(LogTemp, Error, TEXT("Premature detonation triggered by Hugoniot impact shock!"));
			return;
		}

		double t = 0.0;
        double obliquity_radians = 0.0;
        double angleOfAttack_radians = 0.0;

		while (current_velocity > 0.0 && !casing_failure && current_depth < fullDepth) {
			while (current_layer_idx < layer_bottom_depths_local.Num() && current_depth >= layer_bottom_depths_local[current_layer_idx]) {
				current_layer_idx++;
			}

			if (current_layer_idx >= TargetPhysicsData.Layers.Num()) {
                UE_LOG(LogTemp, Warning, TEXT("Target Perforated: Projectile completely pierced all target layers."));
				break;
			}

			if (current_layer_idx != last_layer_idx) {
				last_layer_idx = current_layer_idx;
                UE_LOG(LogTemp, Warning, TEXT("  [LAYER BREACH] Pierced into layer: %s"), *TargetPhysicsData.Layers[current_layer_idx].MaterialName);
			}

			const auto& layer = TargetPhysicsData.Layers[current_layer_idx];
			double layerEntryDepth = (current_layer_idx == 0) ? 0.0 : layer_bottom_depths_local[current_layer_idx - 1];
			double squaredVelocity = current_velocity * current_velocity;
			double baseStrength = layer.CompressiveStrength + (layer.RebarYieldStrength * layer.RebarVolumeFraction);
			double baseDensity = layer.Density;

			if ((current_depth - layerEntryDepth) < layer.PulverizedDepth) {
				baseStrength = 5.0e6;
				baseDensity = layer.Density * 0.7;
			}

			double dynamic_pressure = 0.5 * baseDensity * squaredVelocity;
			if (dynamic_pressure > max_dynamic_pressure) {
				max_dynamic_pressure = dynamic_pressure;
			}

			if (!erosion_active && BombData.YieldStrength > 0.0 && dynamic_pressure >= BombData.YieldStrength) {
				erosion_active = true;
                UE_LOG(LogTemp, Error, TEXT("  [WAPM EROSION ONSET] Hydrodynamic pressure exceeded casing yield at Depth: %.2f m | Velocity: %.2f m/s"), current_depth, current_velocity);
			}

			auto derivative = [&](double v, double z, double theta, double T, double L, double m) -> FPenDeriv {
				FPenDeriv d;
				double vSq = v * v;
				double strain_rate = FMath::Abs(v) / FMath::Max(0.01, BombData.Diameter);
				double dif = ComputeDIF(strain_rate, baseStrength);
				double effective_strength = baseStrength * dif;

				double lateral_force = 0.0;
				double safeMass = FMath::Max(0.001, m);
				double gravity_component = cons.gravity * FMath::Cos(theta);

				if (!erosion_active) {
					double fc_mpa = FMath::Max(0.001, effective_strength / 1.0e6);
					double S = 82.6 * FMath::Pow(fc_mpa, -0.544);
					double CRH_val = (BombData.Diameter > 0.0) ? (BombData.CurvatureNoseReduce / BombData.Diameter) : 3.0;
					double dragCoef = (8.0 * ((CRH_val > 0.0) ? CRH_val : 3.0) - 1.0) / (24.0 * FMath::Pow(((CRH_val > 0.0) ? CRH_val : 3.0), 2));

					double tunnelForce = area * (S * effective_strength + dragCoef * baseDensity * vSq);
					double craterDepthLimit = 2.0 * BombData.Diameter;
					double zLocal = z - layerEntryDepth;
					double axialForce = (craterDepthLimit > 0.0 && zLocal < craterDepthLimit) ? tunnelForce * FMath::Clamp(zLocal / craterDepthLimit, 0.0, 1.0) : tunnelForce;

					d.dv = gravity_component - (axialForce / safeMass);
					d.dz = v * FMath::Cos(theta);
					d.dL = 0.0;
					double heat_rate = (axialForce * cons.frictionFactor) * FMath::Abs(v);
					d.dT = heat_rate / FMath::Max(1.0, (safeMass * BombData.SpecificHeat));
				} else {
					double u = SolveInterfaceVelocity(v, BombData.CasingDensity, baseDensity, BombData.YieldStrength, effective_strength);
					double Le = FMath::Max(0.01, L);
					d.dv = -(BombData.YieldStrength / (BombData.CasingDensity * Le)) * (1.0 + (v - u) / bar_wave_speed) + gravity_component;
					d.dz = u * FMath::Cos(theta);
					d.dL = -(v - u);
					double erosion_heat_rate = 0.5 * baseDensity * (v - u) * (v - u) * area * FMath::Abs(v - u);
					d.dT = erosion_heat_rate / FMath::Max(1.0, (safeMass * BombData.SpecificHeat));
				}

				if (v > 0.1) {
					d.dtheta = (lateral_force / safeMass) / v;
				}
				return d;
			};

			auto get_mass = [&](double L_eval) {
				return erosion_active ? (BombData.TotalMass / FMath::Max(0.01, BombData.Length)) * FMath::Max(0.0, L_eval) : current_mass;
			};

			FPenDeriv k1 = derivative(current_velocity, current_depth, obliquity_radians, current_temperature, current_length, get_mass(current_length));
			FPenDeriv k2 = derivative(current_velocity + 0.5 * dt * k1.dv, current_depth + 0.5 * dt * k1.dz, obliquity_radians + 0.5 * dt * k1.dtheta, current_temperature + 0.5 * dt * k1.dT, current_length + 0.5 * dt * k1.dL, get_mass(current_length + 0.5 * dt * k1.dL));
			FPenDeriv k3 = derivative(current_velocity + 0.5 * dt * k2.dv, current_depth + 0.5 * dt * k2.dz, obliquity_radians + 0.5 * dt * k2.dtheta, current_temperature + 0.5 * dt * k2.dT, current_length + 0.5 * dt * k2.dL, get_mass(current_length + 0.5 * dt * k2.dL));
			FPenDeriv k4 = derivative(current_velocity + dt * k3.dv, current_depth + dt * k3.dz, obliquity_radians + dt * k3.dtheta, current_temperature + dt * k3.dT, current_length + dt * k3.dL, get_mass(current_length + dt * k3.dL));

			double acceleration = k1.dv;
			current_velocity += (dt / 6.0) * (k1.dv + 2 * k2.dv + 2 * k3.dv + k4.dv);
			current_depth += (dt / 6.0) * (k1.dz + 2 * k2.dz + 2 * k3.dz + k4.dz);
			obliquity_radians += (dt / 6.0) * (k1.dtheta + 2 * k2.dtheta + 2 * k3.dtheta + k4.dtheta);
			current_temperature += (dt / 6.0) * (k1.dT + 2 * k2.dT + 2 * k3.dT + k4.dT);
			current_length += (dt / 6.0) * (k1.dL + 2 * k2.dL + 2 * k3.dL + k4.dL);

			FThermalMassAblationResult TMAR = ThermalMassAblation(erosion_active, current_temperature, current_mass, current_length, BombData);
			if (TMAR.casing_failure) {
				casing_failure = true;
			}
			if (TMAR.should_break) {
				break;
			}

			if (current_depth >= next_print_depth) {
				double g_force = acceleration / cons.gravity;
                FString isEroding = erosion_active ? TEXT(" | [ERODING]") : TEXT("");
				UE_LOG(LogTemp, Warning, TEXT("  [Penetration T+ %.2f ms] Depth: %.1f m | Vel: %.1f m/s | Decel: %.0f G | Temp: %.0f K | Layer: %s %s"), (t * 1000.0), current_depth, current_velocity, g_force, current_temperature, *layer.MaterialName, *isEroding);
				next_print_depth += 1.0;
			}
			t += dt;
			if (t > 10.0) break;
		}

		if (current_velocity <= 0.0) {
			FString final_layer = current_layer_idx < TargetPhysicsData.Layers.Num() ? TargetPhysicsData.Layers[current_layer_idx].MaterialName : TEXT("Unknown");
			UE_LOG(LogTemp, Warning, TEXT("  [FULL STOP at T+ %.2f ms] Projectile came to rest at Depth: %.2f m inside layer: %s"), (t * 1000.0), current_depth, *final_layer);
		}

        UE_LOG(LogTemp, Warning, TEXT("------------------------------------"));
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
