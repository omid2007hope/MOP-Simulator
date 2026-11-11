/*
 * Copyright (c) 2026 Omid Teimory. All Rights Reserved.
 *
 * NOTICE: This file contains proprietary source code licensed exclusively under the
 * terms of the LICENSE file in the root directory of this repository.
 * You may view, download, and compile this code solely to run the unmodified product.
 * Copying, pasting, modifying, redistributing, creating derivative works, or using
 * this code in any other project or AI training system is strictly prohibited.
 * Removal or alteration of this notice is a violation of 17 U.S.C. § 1202 and EU law.
 */

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

// Flight Controller Trim
double flightControlTrim(double flightPathAngle,
			 double bomberVelocity,
			 double bombMass,
			 double CurveSlop,
			 double wingArea,
			 double airDensity,
			 const PhysicsConstants& cons);

} // namespace EnvironmentPhysics

#endif // ENVIRONMENT_PHYSICS_HPP
