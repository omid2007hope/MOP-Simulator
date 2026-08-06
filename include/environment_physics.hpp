// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#ifndef ENVIRONMENT_PHYSICS_HPP
#define ENVIRONMENT_PHYSICS_HPP

// files
#include "simulation.hpp"


// ! Namespace for environmental calculations, Hugoniot equations of state, and dynamic material response
namespace EnvironmentPhysics {

double getMachDependentDrag(double mach,
			    double baseCd,
			    const Projectile& proj,
			    const PhysicsConstants& cons);

AtmosphereState standardAtmosphere(double altitude_m, const PhysicsConstants& cons);

double computeDIF(double strain_rate_per_s, double fc_static_pa);

double solveInterfaceVelocity(double v, double rho_p, double rho_t, double Yp, double Rt);

double solveHugoniotInterfaceVelocity(
	double v, double rho_t, double C0_t, double S_t, double rho_p, double C0_p, double S_p);

double flightControlTrim(const AtmosphereState& atmos,
			 const ImpactScenario& scenario,
			 const Projectile& proj,
			 const Aircraft& bomber);

} // namespace EnvironmentPhysics

#endif // ENVIRONMENT_PHYSICS_HPP
