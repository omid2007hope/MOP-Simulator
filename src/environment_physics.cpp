// Copyright (c) 2026 Omid Teimory. All Rights Reserved

// packages
#include <algorithm>
#include <cmath>

// files
#include "environment_physics.hpp"


namespace EnvironmentPhysics {

// ! Calculates Mach dependent aerodynamic drag coefficient based on G7 reference model
double getMachDependentDrag(double mach,
			    double baseCd,
			    const Projectile& proj,
			    const PhysicsConstants& cons) {
	// Standard G7 reference drag function (boat-tailed long-rod/bomb shape; public-domain
	// McCoy/US-Army tabulated data), scaled by this projectile's CRH-derived form factor and
	// blended into modified-Newtonian hypersonic theory above Mach 5 (needed for orbital
	// kinetic strikes at Mach 10-22, well beyond the G7 table's validated range).
	static constexpr double g7Mach[] = {0.00, 0.20, 0.40, 0.60, 0.70, 0.80, 0.85, 0.90,
					    0.95, 1.00, 1.05, 1.10, 1.20, 1.30, 1.40, 1.50,
					    1.70, 1.90, 2.10, 2.30, 2.50, 2.70, 2.90, 3.10,
					    3.30, 3.50, 3.70, 3.90, 4.20, 4.60, 5.00};

	static constexpr double g7Cd[] = {0.1198, 0.1193, 0.1193, 0.1194, 0.1197, 0.1226, 0.1266,
					  0.1368, 0.1660, 0.2993, 0.4015, 0.4034, 0.3919, 0.3785,
					  0.3660, 0.3550, 0.3363, 0.3213, 0.3099, 0.3007, 0.2931,
					  0.2864, 0.2806, 0.2754, 0.2709, 0.2668, 0.2632, 0.2599,
					  0.2557, 0.2508, 0.2465};

	constexpr size_t g7Points = sizeof(g7Mach) / sizeof(g7Mach[0]);

	double formFactor = baseCd / g7Cd[0];

	double clampedMach = std::clamp(mach, g7Mach[0], g7Mach[g7Points - 1]);
	double g7Value = g7Cd[0];
	for (size_t i = 0; i + 1 < g7Points; ++i) {
		// comment why if -- locate current Mach interval in G7 drag table
		if (clampedMach >= g7Mach[i] && clampedMach <= g7Mach[i + 1]) {
			double t = (clampedMach - g7Mach[i]) / (g7Mach[i + 1] - g7Mach[i]);
			g7Value = g7Cd[i] + t * (g7Cd[i + 1] - g7Cd[i]);
			break;
		}
	}
	double cdFromTable = formFactor * g7Value;

	// comment why if -- return standard tabular drag below Mach 5 boundary
	if (mach <= 5.0) {
		return cdFromTable;
	}

	// Modified-Newtonian hypersonic asymptote from the projectile's own nose geometry:
	// treat the ogive nose as an equivalent cone (Cd = 2*sin^2(half-angle), Newtonian limit).
	double CRH = (proj.diameter > 0.0) ? (proj.curvature_noseReduce / proj.diameter) : 3.0;
	double effectiveCRH = (CRH > 0.25) ? CRH : 3.0;
	double noseLength = proj.diameter * std::sqrt(effectiveCRH - 0.25);
	double noseHalfAngle = std::atan((proj.diameter / 2.0) / std::max(1e-6, noseLength));
	double cdNewtonian = 2.0 * std::pow(std::sin(noseHalfAngle), 2);

	// comment why if -- use full Newtonian hypersonic drag above Mach 8
	if (mach >= 8.0) {
		return cdNewtonian;
	}

	// Smooth cosine blend from the G7 table's Mach-5 value into the hypersonic asymptote.
	double blend = 0.5 * (1.0 - std::cos(cons.PI * (mach - 5.0) / 3.0));
	return cdFromTable + blend * (cdNewtonian - cdFromTable);
	// **** Ends Here ****
}




// ! Evaluates standard US 1976 atmospheric properties based on geometric altitude
AtmosphereState standardAtmosphere(double altitude_m, const PhysicsConstants& cons) {
	// US Standard Atmosphere 1976: piecewise layers defined by geopotential height.
	struct AtmosphereLayer {
		double base_geopotential_m;
		double base_temperature_K;
		double base_pressure_Pa;
		double lapse_rate_Kpm;
	};
	static constexpr AtmosphereLayer layers[] = {
		{0.0, 288.15, 101325.0, -0.0065},    // Troposphere
		{11000.0, 216.65, 22632.1, 0.0},     // Tropopause
		{20000.0, 216.65, 5474.89, 0.0010},  // Stratosphere 1
		{32000.0, 228.65, 868.019, 0.0028},  // Stratosphere 2
		{47000.0, 270.65, 110.906, 0.0},     // Stratopause
		{51000.0, 270.65, 66.9389, -0.0028}, // Mesosphere 1
		{71000.0, 214.65, 3.95642, -0.0020}, // Mesosphere 2
	};
	constexpr size_t layerCount = sizeof(layers) / sizeof(layers[0]);
	constexpr double topGeopotential_m = 84852.0; // Last defined layer boundary

	// Geopotential height h = r*Z / (r+Z)
	double h = (cons.earthRadius * altitude_m) / (cons.earthRadius + altitude_m);
	double h_eval = std::min(h, topGeopotential_m);

	size_t idx = 0;
	for (size_t i = 0; i < layerCount; ++i) {
		// comment why if -- find active atmospheric layer boundary
		if (h_eval >= layers[i].base_geopotential_m) {
			idx = i;
		}
	}
	const AtmosphereLayer& base = layers[idx];

	double g0M_over_R = (cons.gravity * cons.molarMassAir) / cons.universalGasConstant;
	double T, P;
	// comment why if -- calculate lapse rate temperature vs isothermal pressure
	if (std::fabs(base.lapse_rate_Kpm) > 1e-12) {
		T = base.base_temperature_K +
		    base.lapse_rate_Kpm * (h_eval - base.base_geopotential_m);
		P = base.base_pressure_Pa *
		    std::pow(base.base_temperature_K / T, g0M_over_R / base.lapse_rate_Kpm);
	} else {
		T = base.base_temperature_K;
		P = base.base_pressure_Pa *
		    std::exp(-g0M_over_R * (h_eval - base.base_geopotential_m) / T);
	}

	// Beyond the last defined layer (~85 km): continue isothermal exponential decay.
	// This is the start of the thermosphere/free-molecular regime; density is already
	// negligible for drag purposes, so a smooth continued extrapolation is adequate.
	// comment why if -- decay pressure beyond thermosphere ceiling
	if (h > topGeopotential_m) {
		P *= std::exp(-g0M_over_R * (h - topGeopotential_m) / T);
	}

	AtmosphereState state;
	state.temperature_K = T;
	state.pressure_Pa = P;
	state.density_kgm3 = (P * cons.molarMassAir) / (cons.universalGasConstant * T);
	state.speed_of_sound_ms = std::sqrt(
		(cons.adiabaticIndexAir * cons.universalGasConstant * T) / cons.molarMassAir);
	return state;
	// **** Ends Here ****
}




// ! Computes CEB-FIP dynamic increase factor for target strength under high strain rate
double computeDIF(double strain_rate_per_s, double fc_static_pa) {
	// CEB-FIP Model Code 1990/2010 Dynamic Increase Factor for compressive strength.
	constexpr double staticReferenceStrainRate = 30.0e-6; // 1/s
	constexpr double referenceStress_fco = 10.0e6;	      // Pa (10 MPa)

	double fc_static = std::max(1.0e5, fc_static_pa); // avoid div-by-zero for very weak layers
	double alpha = 1.0 / (5.0 + 9.0 * (fc_static / referenceStress_fco));
	double strain_rate = std::max(strain_rate_per_s, staticReferenceStrainRate);

	// comment why if -- evaluate low strain rate dynamic amplification curve
	if (strain_rate <= 30.0) {
		return std::pow(strain_rate / staticReferenceStrainRate, 1.026 * alpha);
	}

	double logGamma = 6.156 * alpha - 2.0;
	double gamma = std::pow(10.0, logGamma);
	return gamma * std::pow(strain_rate / staticReferenceStrainRate, 1.0 / 3.0);
	// **** Ends Here ****
}




// ! Solves Tate-Bernoulli hydrodynamic interface erosion velocity
double solveInterfaceVelocity(double v, double rho_p, double rho_t, double Yp, double Rt) {
	// Tate-Bernoulli quasi-steady balance at the eroding interface:
	//   0.5*rho_p*(v-u)^2 + Yp = 0.5*rho_t*u^2 + Rt
	// Rearranged into A*u^2 + B*u + C = 0 and solved analytically each call.
	double A = 0.5 * (rho_p - rho_t);
	double B = -rho_p * v;
	double C = 0.5 * rho_p * v * v - (Rt - Yp);

	// comment why if -- solve linear form when density match eliminates quadratic term
	if (std::fabs(A) < 1.0e-6) {
		// comment why if -- avoid div-by-zero on zero velocity
		if (std::fabs(B) < 1.0e-9) {
			return 0.0;
		}
		return std::clamp(-C / B, 0.0, v);
	}

	double discriminant = B * B - 4.0 * A * C;
	// comment why if -- zero interface speed if discriminant negative
	if (discriminant < 0.0) {
		return 0.0; // No physically valid erosion interface; treat as fully rigid this step
	}

	double sqrtDisc = std::sqrt(discriminant);
	double root1 = (-B + sqrtDisc) / (2.0 * A);
	double root2 = (-B - sqrtDisc) / (2.0 * A);

	bool root1Valid = (root1 >= 0.0 && root1 <= v);
	bool root2Valid = (root2 >= 0.0 && root2 <= v);

	// comment why if -- choose valid physical interface velocity root
	if (root1Valid && root2Valid) {
		return std::min(root1, root2);
	}
	// comment why if -- root 1 valid check
	if (root1Valid) {
		return root1;
	}
	// comment why if -- root 2 valid check
	if (root2Valid) {
		return root2;
	}
	return 0.0;
	// **** Ends Here ****
}




// ! Solves Hugoniot shock impedance matching interface velocity
double solveHugoniotInterfaceVelocity(
	double v, double rho_t, double C0_t, double S_t, double rho_p, double C0_p, double S_p) {
	// Shock impedance matching at the target/casing interface: continuity of pressure and
	// particle velocity between the target's Hugoniot (shocked from rest) and the projectile's
	// Hugoniot (decelerated from v), solved analytically for the common interface velocity x.
	double A = rho_t * S_t - rho_p * S_p;
	double B = rho_t * C0_t + rho_p * C0_p + 2.0 * rho_p * S_p * v;
	double C = -rho_p * v * (C0_p + S_p * v);

	// comment why if -- handle matched Hugoniot linear solution limit
	if (std::fabs(A) < 1.0e-6) {
		// comment why if -- avoid div-by-zero on zero impact speed
		if (std::fabs(B) < 1.0e-9) {
			return 0.0;
		}
		return std::clamp(-C / B, 0.0, v);
	}

	double discriminant = B * B - 4.0 * A * C;
	// comment why if -- check real shock solution existence
	if (discriminant < 0.0) {
		return 0.0;
	}

	double sqrtDisc = std::sqrt(discriminant);
	double root1 = (-B + sqrtDisc) / (2.0 * A);
	double root2 = (-B - sqrtDisc) / (2.0 * A);

	bool root1Valid = (root1 >= 0.0 && root1 <= v);
	bool root2Valid = (root2 >= 0.0 && root2 <= v);

	// comment why if -- pick lower physical particle velocity root
	if (root1Valid && root2Valid) {
		return std::min(root1, root2);
	}
	// comment why if -- check root 1
	if (root1Valid) {
		return root1;
	}
	// comment why if -- check root 2
	if (root2Valid) {
		return root2;
	}
	return 0.0;
	// **** Ends Here ****
}




// ! physics-based Aircraft Flight Control & Trim system
double flightControlTrim(const AtmosphereState& atmos,
			 const ImpactScenario& scenario,
			 const Projectile& proj,
			 const Aircraft& bomber,
			 const PhysicsConstants& cons) {

	Aircraft spec;
	spec.bomber_totalMass;
	spec.bomber_wingArea;
	spec.bomber_liftCurveSlope;

	Projectile bomb;
	bomb.total_mass;

	ImpactScenario sce;
	sce.velocity;
	sce.flight_path_angle;

	// Bombers velocity is as same as inital velocity
	double bomberVelocity = sce.velocity;

	AtmosphereState atm = standardAtmosphere(altitude_m, cons);
	double density = atm.density_kgm3;

	double fpa_rad = sce.flight_path_angle * cons.PI / 180.0;

	double delta_lift = -(spec.total_mass * cons.gravity * std::cos(fpa_rad));

	double dynamic_pressure = 0.5 * density * std::pow(bomberVelocity, 2);

	// 6. Required Change in Lift Coefficient (Delta CL)
	double delta_CL = delta_lift / (dynamic_pressure * spec.bomber_wingArea);

	// 7. Required Trim Angle Change (Degrees)
	spec.delta_alpha_deg = delta_CL / spec.bomber_liftCurveSlope;

	return spec.delta_alpha_deg // This will be a negative number
				    // (Nose Down)
};
} // namespace EnvironmentPhysics
