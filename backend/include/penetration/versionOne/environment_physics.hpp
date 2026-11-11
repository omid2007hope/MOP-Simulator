// SPDX-License-Identifier: LicenseRef-MopSimulator-Proprietary
/*
 * Copyright (c) 2026 Omid Teimory. All Rights Reserved.
 *
 * PROPRIETARY SOURCE-AVAILABLE SOFTWARE — NOT OPEN SOURCE.
 * Permission to view, clone, or compile this file is granted strictly on the
 * condition precedent of full compliance with the root ./LICENSE file.
 * Copying snippets, modifying, creating derivative works, redistributing,
 * commercial/SaaS use, and AI/ML training (reserved under EU Directive
 * 2019/790 Art. 4(3) and § 42h(6) Austrian UrhG) are strictly prohibited.
 * Removal or alteration of this notice violates 17 U.S.C. § 1202 and § 90c UrhG.
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
