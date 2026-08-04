// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#include "simulation.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

ImpactSimulator::ImpactSimulator(const Projectile& p, const Target& t, const PhysicsConstants& c)
    : proj(p), target(t), cons(c)
{
}

std::pair<double, double> ImpactSimulator::computeProjectileRotationInAir(double horizontal_velocity,
                                                                        double vertical_velocity,
                                                                        double drag_coef) const
{
    // 2DOF Translation Dynamics: Computes horizontal and vertical accelerations
    // based on drag force, mass, gravity, and current velocity vector.
    double gamma = std::atan2(horizontal_velocity, vertical_velocity); // Flight path angle

    double x_acceleration = -(drag_coef * std::sin(gamma)) / proj.total_mass;
    double y_acceleration = cons.gravity - (drag_coef * std::cos(gamma)) / proj.total_mass;
    
    return {x_acceleration, y_acceleration};
}

double ImpactSimulator::getMachDependentDrag(double mach, double baseCd) const
{
    // Standard G7 reference drag function (boat-tailed long-rod/bomb shape; public-domain
    // McCoy/US-Army tabulated data), scaled by this projectile's CRH-derived form factor and
    // blended into modified-Newtonian hypersonic theory above Mach 5 (needed for orbital
    // kinetic strikes at Mach 10-22, well beyond the G7 table's validated range).
    static constexpr double g7Mach[] = {0.00, 0.20, 0.40, 0.60, 0.70, 0.80, 0.85, 0.90,
                                        0.95, 1.00, 1.05, 1.10, 1.20, 1.30, 1.40, 1.50,
                                        1.70, 1.90, 2.10, 2.30, 2.50, 2.70, 2.90, 3.10,
                                        3.30, 3.50, 3.70, 3.90, 4.20, 4.60, 5.00};

    static constexpr double g7Cd[] = {
        0.1198, 0.1193, 0.1193, 0.1194, 0.1197, 0.1226, 0.1266, 0.1368, 0.1660, 0.2993, 0.4015,
        0.4034, 0.3919, 0.3785, 0.3660, 0.3550, 0.3363, 0.3213, 0.3099, 0.3007, 0.2931, 0.2864,
        0.2806, 0.2754, 0.2709, 0.2668, 0.2632, 0.2599, 0.2557, 0.2508, 0.2465};

    constexpr size_t g7Points = sizeof(g7Mach) / sizeof(g7Mach[0]);

    double formFactor = baseCd / g7Cd[0];

    double clampedMach = std::clamp(mach, g7Mach[0], g7Mach[g7Points - 1]);
    double g7Value = g7Cd[0];
        for (size_t i = 0; i + 1 < g7Points; ++i) {
                if (clampedMach >= g7Mach[i] && clampedMach <= g7Mach[i + 1]) {
                    double t = (clampedMach - g7Mach[i]) / (g7Mach[i + 1] - g7Mach[i]);
                    g7Value = g7Cd[i] + t * (g7Cd[i + 1] - g7Cd[i]);
                    break;
                }
        }
    double cdFromTable = formFactor * g7Value;

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

        if (mach >= 8.0) {
            return cdNewtonian;
        }

    // Smooth cosine blend from the G7 table's Mach-5 value into the hypersonic asymptote.
    double blend = 0.5 * (1.0 - std::cos(cons.PI * (mach - 5.0) / 3.0));
    return cdFromTable + blend * (cdNewtonian - cdFromTable);
}

AtmosphereState ImpactSimulator::standardAtmosphere(double altitude_m) const
{
    // US Standard Atmosphere 1976: piecewise layers defined by geopotential height.
    struct AtmosphereLayer
    {
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
                if (h_eval >= layers[i].base_geopotential_m) {
                    idx = i;
                }
        }
    const AtmosphereLayer& base = layers[idx];

    double g0M_over_R = (cons.gravity * cons.molarMassAir) / cons.universalGasConstant;
    double T, P;
        if (std::fabs(base.lapse_rate_Kpm) > 1e-12) {
            T = base.base_temperature_K + base.lapse_rate_Kpm * (h_eval - base.base_geopotential_m);
            P = base.base_pressure_Pa *
                std::pow(base.base_temperature_K / T, g0M_over_R / base.lapse_rate_Kpm);
        }
        else {
            T = base.base_temperature_K;
            P = base.base_pressure_Pa *
                std::exp(-g0M_over_R * (h_eval - base.base_geopotential_m) / T);
        }

        // Beyond the last defined layer (~85 km): continue isothermal exponential decay.
        // This is the start of the thermosphere/free-molecular regime; density is already
        // negligible for drag purposes, so a smooth continued extrapolation is adequate.
        if (h > topGeopotential_m) {
            P *= std::exp(-g0M_over_R * (h - topGeopotential_m) / T);
        }

    AtmosphereState state;
    state.temperature_K = T;
    state.pressure_Pa = P;
    state.density_kgm3 = (P * cons.molarMassAir) / (cons.universalGasConstant * T);
    state.speed_of_sound_ms =
        std::sqrt((cons.adiabaticIndexAir * cons.universalGasConstant * T) / cons.molarMassAir);
    return state;
}

double ImpactSimulator::computeDIF(double strain_rate_per_s, double fc_static_pa)
{
    // CEB-FIP Model Code 1990/2010 Dynamic Increase Factor for compressive strength.
    constexpr double staticReferenceStrainRate = 30.0e-6; // 1/s
    constexpr double referenceStress_fco = 10.0e6;        // Pa (10 MPa)

    double fc_static = std::max(1.0e5, fc_static_pa); // avoid div-by-zero for very weak layers
    double alpha = 1.0 / (5.0 + 9.0 * (fc_static / referenceStress_fco));
    double strain_rate = std::max(strain_rate_per_s, staticReferenceStrainRate);

        if (strain_rate <= 30.0) {
            return std::pow(strain_rate / staticReferenceStrainRate, 1.026 * alpha);
        }

    double logGamma = 6.156 * alpha - 2.0;
    double gamma = std::pow(10.0, logGamma);
    return gamma * std::pow(strain_rate / staticReferenceStrainRate, 1.0 / 3.0);
}

double ImpactSimulator::solveInterfaceVelocity(
    double v, double rho_p, double rho_t, double Yp, double Rt) const
{
    // Tate-Bernoulli quasi-steady balance at the eroding interface:
    //   0.5*rho_p*(v-u)^2 + Yp = 0.5*rho_t*u^2 + Rt
    // Rearranged into A*u^2 + B*u + C = 0 and solved analytically each call.
    double A = 0.5 * (rho_p - rho_t);
    double B = -rho_p * v;
    double C = 0.5 * rho_p * v * v - (Rt - Yp);

        if (std::fabs(A) < 1.0e-6) {
                if (std::fabs(B) < 1.0e-9) {
                    return 0.0;
                }
            return std::clamp(-C / B, 0.0, v);
        }

    double discriminant = B * B - 4.0 * A * C;
        if (discriminant < 0.0) {
            return 0.0; // No physically valid erosion interface; treat as fully rigid this step
        }

    double sqrtDisc = std::sqrt(discriminant);
    double root1 = (-B + sqrtDisc) / (2.0 * A);
    double root2 = (-B - sqrtDisc) / (2.0 * A);

    bool root1Valid = (root1 >= 0.0 && root1 <= v);
    bool root2Valid = (root2 >= 0.0 && root2 <= v);

        if (root1Valid && root2Valid) {
            return std::min(root1, root2);
        }
        if (root1Valid) {
            return root1;
        }
        if (root2Valid) {
            return root2;
        }
    return 0.0;
}

double ImpactSimulator::solveHugoniotInterfaceVelocity(
    double v, double rho_t, double C0_t, double S_t, double rho_p, double C0_p, double S_p) const
{
    // Shock impedance matching at the target/casing interface: continuity of pressure and
    // particle velocity between the target's Hugoniot (shocked from rest) and the projectile's
    // Hugoniot (decelerated from v), solved analytically for the common interface velocity x.
    double A = rho_t * S_t - rho_p * S_p;
    double B = rho_t * C0_t + rho_p * C0_p + 2.0 * rho_p * S_p * v;
    double C = -rho_p * v * (C0_p + S_p * v);

        if (std::fabs(A) < 1.0e-6) {
                if (std::fabs(B) < 1.0e-9) {
                    return 0.0;
                }
            return std::clamp(-C / B, 0.0, v);
        }

    double discriminant = B * B - 4.0 * A * C;
        if (discriminant < 0.0) {
            return 0.0;
        }

    double sqrtDisc = std::sqrt(discriminant);
    double root1 = (-B + sqrtDisc) / (2.0 * A);
    double root2 = (-B - sqrtDisc) / (2.0 * A);

    bool root1Valid = (root1 >= 0.0 && root1 <= v);
    bool root2Valid = (root2 >= 0.0 && root2 <= v);

        if (root1Valid && root2Valid) {
            return std::min(root1, root2);
        }
        if (root1Valid) {
            return root1;
        }
        if (root2Valid) {
            return root2;
        }
    return 0.0;
}

SimulationResult ImpactSimulator::simulate(const ImpactScenario& scenario)
{
    SimulationResult res;
    res.scenario_name = scenario.name;
    res.altitude_ft = scenario.altitude_ft;
    res.velocity = scenario.velocity;
    res.mach_number =
        scenario.velocity / standardAtmosphere(scenario.altitude_ft / 3.28084).speed_of_sound_ms;

    res.casing_failure = false;
    res.premature_detonation = false;
    res.explosive_charge_survives = true;
    res.shock_damage_prob_percent = 0.0;
    res.regime = "Rigid Penetration (Crater+Tunnel)";
    res.outcome_summary = "Intact";

    double current_velocity = scenario.velocity;
    double current_mass = proj.total_mass;
    double current_depth = 0.0;

    // Obliquity and AoA
    double obliquity_radians = scenario.obliquity_angle * cons.PI / 180.0;
    double angleOfAttack_radians = scenario.angle_of_attack * cons.PI / 180.0;

    double dt = 1e-5; // 10 microseconds
    double t = 0.0;
    double current_temperature = 300.0; // Kelvin

    double max_dynamic_pressure = 0.0;
    double max_walker_wasley_product = 0.0; // peak P^2*tau (Pa^2*s) across all shock events

    double area = cons.PI * std::pow(proj.diameter / 2.0, 2);

    double CRH = 0.0;
        if (proj.diameter > 0.0) {
            CRH = proj.curvature_noseReduce / proj.diameter;
        }

    // Calculate Nose Performance Coefficient (N)
    double Caliber_Radius_Head = (CRH > 0.0) ? CRH : 3.0;

    double dragCoefficient =
        (8.0 * Caliber_Radius_Head - 1.0) / (24.0 * std::pow(Caliber_Radius_Head, 2));

    double dropAltitude = scenario.altitude_ft;
    double current_altitude = dropAltitude;

        if (dropAltitude > 0.0) {
            std::cout << "\n--- Simulating Atmospheric Drop from " << dropAltitude << " ft ---\n";
            double dt_drop = 0.01; // 10ms for drop integration
            double next_print_altitude = dropAltitude - 5000.0;
            bool sonic_boom_triggered = false;
            double t_drop = 0.0;
            int drop_frame_counter = 0;

            double y_m = current_altitude / 3.28084;

                while (current_altitude > 0.0) {
                    auto calc_accel = [&](double alt_m, double vel) {
                        AtmosphereState atm = standardAtmosphere(alt_m);
                        double mach = vel / atm.speed_of_sound_ms;
                        double cd = getMachDependentDrag(mach, dragCoefficient);
                        double f = 0.5 * atm.density_kgm3 * vel * vel * cd * area;
                        return cons.gravity - (f / proj.total_mass);
                    };

                    double k1_v = calc_accel(y_m, current_velocity);
                    double k1_y = -current_velocity;

                    double k2_v = calc_accel(y_m + 0.5 * dt_drop * k1_y,
                                             current_velocity + 0.5 * dt_drop * k1_v);
                    double k2_y = -(current_velocity + 0.5 * dt_drop * k1_v);

                    double k3_v = calc_accel(y_m + 0.5 * dt_drop * k2_y,
                                             current_velocity + 0.5 * dt_drop * k2_v);
                    double k3_y = -(current_velocity + 0.5 * dt_drop * k2_v);

                    double k4_v =
                        calc_accel(y_m + dt_drop * k3_y, current_velocity + dt_drop * k3_v);
                    double k4_y = -(current_velocity + dt_drop * k3_v);

                    double prev_y_m = y_m;
                    double prev_vel = current_velocity;

                    current_velocity += (dt_drop / 6.0) * (k1_v + 2 * k2_v + 2 * k3_v + k4_v);
                    y_m += (dt_drop / 6.0) * (k1_y + 2 * k2_y + 2 * k3_y + k4_y);

                        if (y_m < 0.0) {
                            double fraction = prev_y_m / (prev_y_m - y_m);
                            current_velocity = prev_vel + fraction * (current_velocity - prev_vel);
                            y_m = 0.0;
                            t_drop = t_drop - dt_drop + fraction * dt_drop;
                        }

                    current_altitude = y_m * 3.28084;
                    AtmosphereState current_atm = standardAtmosphere(y_m);
                    double current_density = current_atm.density_kgm3;

                    bool is_sonic_boom_frame = false;
                        if (current_velocity >= current_atm.speed_of_sound_ms &&
                            !sonic_boom_triggered) {
                            sonic_boom_triggered = true;
                            is_sonic_boom_frame = true;
                            std::cout << "  >>> [SONIC BOOM] Mach 1 exceeded at T+ " << std::fixed
                                      << std::setprecision(2) << t_drop
                                      << "s (Altitude: " << std::setprecision(0) << current_altitude
                                      << " ft | Density: " << std::setprecision(3)
                                      << current_density << " kg/m^3) <<<\n";
                        }

                        // Save telemetry frame every 10 steps (0.1s) or on sonic boom to keep JSON
                        // size reasonable
                        if (drop_frame_counter++ % 10 == 0 || is_sonic_boom_frame) {
                            TelemetryFrame frame;
                            frame.time = t_drop;
                            frame.altitude =
                                current_altitude / 3.28084; // Store altitude in meters for WebGL
                            frame.depth = -frame.altitude;  // Visualizer expects depth, so sky is
                                                            // negative depth
                            frame.velocity = current_velocity;
                            frame.mach = current_velocity / current_atm.speed_of_sound_ms;
                            frame.is_sonic_boom = is_sonic_boom_frame;
                            res.drop_frames.push_back(frame);
                        }

                        if (current_altitude <= next_print_altitude && current_altitude > 0.0) {
                            double acceleration = calc_accel(y_m, current_velocity);
                            double g_force = acceleration / cons.gravity;
                            std::cout << "  [Drop T+ " << std::fixed << std::setprecision(1)
                                      << t_drop << "s] Alt: " << std::setprecision(0)
                                      << current_altitude << " ft | Vel: " << std::setprecision(1)
                                      << current_velocity
                                      << " m/s | Accel: " << std::setprecision(2) << g_force
                                      << " G | Density: " << std::setprecision(3) << current_density
                                      << " kg/m^3\n";
                            next_print_altitude -= 5000.0;
                        }
                    t_drop += dt_drop;
                }

            current_altitude = 0.0;
            std::cout << "  [Impact T+ " << std::fixed << std::setprecision(2) << t_drop
                      << "s] Alt: 0 ft | Impact Velocity: " << std::setprecision(1)
                      << current_velocity << " m/s\n";
            std::cout << "--------------------------------------------------------\n\n";
        }

    // Update result with final impact velocity (Mach referenced to sea-level speed of sound;
    // Mach number is not a standard concept once inside a solid target)
    // Multi-bomb salvo cumulative shaft tracking: calculate initial breached shaft entry depth
    double initial_shaft_depth = 0.0;
        for (const auto& layer : target.layers) {
                if (layer.pulverized_depth > 0) {
                    initial_shaft_depth += std::min(layer.thickness, layer.pulverized_depth);
                }
        }
    res.previous_strike_depth = initial_shaft_depth;
        if (initial_shaft_depth > 0) {
            current_depth = initial_shaft_depth;
            std::cout << "  [SEQUENTIAL SALVO STRIKE] Entering pre-existing breached shaft depth: "
                      << initial_shaft_depth << " m\n";
        }

    // Convert target layers to fullDepth depths
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
            return res;
        }

    // Walker-Wasley shock initiation (Hugoniot impedance matching): each new material
    // interface encountered (initial impact, and every subsequent layer breach) generates a
    // fresh transient shock loading event into the casing. Track the worst P^2*tau observed.
    auto evaluateShockEvent = [&](double impactVel, const TargetLayer& lyr) {
        double Up = solveHugoniotInterfaceVelocity(impactVel,
                                                   lyr.density,
                                                   lyr.hugoniot_c0,
                                                   lyr.hugoniot_s,
                                                   proj.casing_density,
                                                   proj.hugoniot_c0,
                                                   proj.hugoniot_s);
        double P = lyr.density * (lyr.hugoniot_c0 + lyr.hugoniot_s * Up) * Up;
        double Us_casing = proj.hugoniot_c0 + proj.hugoniot_s * (impactVel - Up);
        double tau = (Us_casing > 1.0) ? (proj.casing_wall_thickness / Us_casing) : 0.0;
        double product = P * P * tau;
            if (product > max_walker_wasley_product) {
                max_walker_wasley_product = product;
                res.shock_pressure_gpa_peak = P / 1.0e9;
                res.shock_pulse_duration_us = tau * 1.0e6;
            }
    };
        if (!target.layers.empty()) {
            evaluateShockEvent(current_velocity, target.layers[0]);
        }

    const double groundSpeedOfSound = standardAtmosphere(0.0).speed_of_sound_ms;
    res.mach_number = current_velocity / groundSpeedOfSound;

    // Time Integration Loop (RK4) - two-phase Forrestal cratering/tunneling with CEB-FIP
    // strain-rate strengthening in the rigid regime, transitioning to the Walker-Anderson
    // erosion model (WAPM) once hydrodynamic pressure overwhelms the casing's dynamic yield.
    double current_length = proj.length;
    bool erosion_active = false;
    double bar_wave_speed = std::sqrt(proj.elastic_modulus / proj.casing_density);
    res.bar_wave_speed = bar_wave_speed;

    struct PenDeriv
    {
        double dv = 0.0;
        double dz = 0.0;
        double dtheta = 0.0;
        double dT = 0.0;
        double dL = 0.0;
    };

        while (current_velocity > 0.0 && !res.casing_failure && current_depth < fullDepth) {
                // Advance layer if we've pierced the current one
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
                    evaluateShockEvent(current_velocity, target.layers[current_layer_idx]);
                }

            const auto& layer = target.layers[current_layer_idx];
            double layerEntryDepth =
                (current_layer_idx == 0) ? 0.0 : layer_bottom_depths[current_layer_idx - 1];

            double squaredVelocity = current_velocity * current_velocity;

            // Pulverized crater zone (from previous strikes) vastly reduces base resistance
            double baseStrength = layer.compressive_strength +
                                  (layer.rebar_yield_strength * layer.rebar_volume_fraction);
            double baseDensity = layer.density;
                if (current_depth < layer.pulverized_depth) {
                    baseStrength = 5.0e6;              // 5 MPa for pulverized rubble
                    baseDensity = layer.density * 0.7; // 30% void fraction
                }

            // Dynamic Pressure (drives shock-initiation tracking and the WAPM transition test)
            double dynamic_pressure = 0.5 * baseDensity * squaredVelocity;
                if (dynamic_pressure > max_dynamic_pressure) {
                    max_dynamic_pressure = dynamic_pressure;
                }

                // Walker & Anderson (1995) transition: once hydrodynamic pressure overwhelms the
                // casing's dynamic yield strength, the rigid-body assumption breaks down and the
                // projectile begins eroding hydrodynamically (latches permanently once triggered).
                if (!erosion_active && proj.yield_strength > 0.0 &&
                    dynamic_pressure >= proj.yield_strength) {
                    erosion_active = true;
                    res.erosion_occurred = true;
                    std::cout << "  [WAPM EROSION ONSET] Hydrodynamic pressure exceeded casing "
                                 "yield at Depth: "
                              << current_depth << " m | Velocity: " << current_velocity << " m/s\n";
                }

            // Obliquity/AoA bending structural failure check (discrete, evaluated pre-step)
            double asymmetric_force = 0.0;
                if (obliquity_radians > 0.0 || angleOfAttack_radians > 0.0) {
                    asymmetric_force = (0.5 * baseDensity * squaredVelocity * area) *
                                       std::sin(obliquity_radians + angleOfAttack_radians);
                    double bending_moment = asymmetric_force * (proj.length / 2.0);

                    double max_bending_stress = 0.0;
                        if (proj.area_moment_inertia > 0) {
                            max_bending_stress =
                                (bending_moment * (proj.diameter / 2.0)) / proj.area_moment_inertia;
                        }

                        if (proj.yield_strength > 0.0 && max_bending_stress > proj.yield_strength) {
                            res.casing_failure = true;
                            res.regime = "Structural Failure (J-Hook/Snap)";
                            res.outcome_summary = "Bending moments exceeded casing yield strength.";
                            break;
                        }
                }

            // RK4 state derivative: (velocity, depth, obliquity, temperature, rigid/eroding length)
            auto derivative =
                [&](double v, double z, double theta, [[maybe_unused]] double T, double L, double m)
                -> PenDeriv {
                PenDeriv d;
                double vSq = v * v;
                double strain_rate = std::fabs(v) / std::max(0.01, proj.diameter);
                double dif = computeDIF(strain_rate, baseStrength);
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
                        // Two-phase Forrestal: linear crater ramp for z_local<=2d (continuous by
                        // construction at z_local=2d), full cavity expansion resistance beyond.
                        double fc_mpa = std::max(0.001, effective_strength / 1.0e6);
                        double S = 82.6 * std::pow(fc_mpa, -0.544);
                        double tunnelForce = area * (S * effective_strength +
                                                     scenario.dragCoefficient * baseDensity * vSq);
                        double craterDepthLimit = 2.0 * proj.diameter;
                        double zLocal = z - layerEntryDepth;
                        double axialForce =
                            (craterDepthLimit > 0.0 && zLocal < craterDepthLimit)
                                ? tunnelForce * std::clamp(zLocal / craterDepthLimit, 0.0, 1.0)
                                : tunnelForce;

                        d.dv = gravity_component - (axialForce / safeMass);
                        d.dz = v * std::cos(theta);
                        d.dL = 0.0;

                        double heat_rate = (axialForce * cons.frictionFactor) * std::fabs(v);
                        d.dT = heat_rate / (safeMass * proj.specific_heat);
                    }
                    else {
                        // Walker-Anderson (WAPM): Tate-Bernoulli interface velocity u, then the
                        // rate-dependent tail deceleration coupling the elastic bar wave speed.
                        double u = solveInterfaceVelocity(v,
                                                          proj.casing_density,
                                                          baseDensity,
                                                          proj.yield_strength,
                                                          effective_strength);
                        double Le =
                            std::max(0.01, L); // elastic length ~= instantaneous rigid length
                        d.dv = -(proj.yield_strength / (proj.casing_density * Le)) *
                                   (1.0 + (v - u) / bar_wave_speed) +
                               gravity_component;
                        d.dz =
                            u * std::cos(theta); // depth tracks the eroding interface, not tail v
                        d.dL = -(v - u);

                        double erosion_heat_rate =
                            0.5 * baseDensity * (v - u) * (v - u) * area * std::fabs(v - u);
                        d.dT = erosion_heat_rate / (safeMass * proj.specific_heat);
                    }

                    if (v > 0.1) {
                        d.dtheta = (lateral_force / safeMass) / v;
                    }

                return d;
            };

            auto get_mass = [&](double L_eval) {
                return erosion_active ? (proj.total_mass / proj.length) * std::max(0.0, L_eval)
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
                    // Thermal ablation mass loss (rigid regime only; the eroding regime derives
                    // mass loss from the eroded length instead, to avoid double-counting).
                        if (current_temperature > proj.melting_point) {
                            double excess_temp = current_temperature - proj.melting_point;
                            double excess_heat = excess_temp * current_mass * proj.specific_heat;

                                if (excess_heat > 0 && proj.heat_of_fusion > 0) {
                                    double mass_loss = excess_heat / proj.heat_of_fusion;
                                    current_mass -= mass_loss;
                                    current_temperature = proj.melting_point; // Clamp temperature

                                        if (current_mass < 0.1 * proj.total_mass) { // Burned up
                                            res.casing_failure = true;
                                            res.regime = "Thermal Destruction";
                                            res.outcome_summary = "Projectile completely ablated.";
                                            break;
                                        }
                                }
                        }
                }
                else {
                    current_length = std::max(0.0, current_length);
                    double effective_linear_density = proj.total_mass / proj.length;
                    current_mass = effective_linear_density * current_length;
                    res.final_rod_length = current_length;
                    res.erosion_length_lost = proj.length - current_length;

                        if (current_length < 0.05 * proj.length) { // Fully eroded/consumed
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

                // Add telemetry frame every 20 iterations (200 microseconds)
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

            // Failsafe for infinite loop (e.g. 10 seconds max)
            if (t > 10.0)
                break;
        }

        if (current_velocity <= 0.0) {
            std::string final_layer = current_layer_idx < target.layers.size()
                                          ? target.layers[current_layer_idx].material_name
                                          : "Unknown";
            std::cout << "  [FULL STOP at T+ " << std::fixed << std::setprecision(2) << (t * 1000.0)
                      << " ms] Projectile came to rest at Depth: " << std::setprecision(2)
                      << current_depth << " m inside layer: " << final_layer << "\n";
        }
    std::cout << "------------------------------------\n\n";

    // Update the pulverized depth for sequential strikes
    target.pulverizeDepth(current_depth);
    res.cumulative_breach_depth = current_depth;

    res.actual_penetration_depth = current_depth;
    res.dynamic_pressure = max_dynamic_pressure;
    res.kinetic_energy = 0.5 * proj.total_mass * std::pow(res.velocity, 2);
    res.rigid_penetration = current_depth; // Backward compat

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

    // Shock Damage
    res.is_kinetic_rod = (proj.explosive_mass == 0.0);

        if (res.is_kinetic_rod) {
                if (res.casing_failure) {
                    // Already failed due to bending, thermal destruction, or erosion burnout
                }
                else if (res.erosion_occurred) {
                    res.regime = "Hypervelocity Erosion (Walker-Anderson)";
                    res.outcome_summary =
                        "Projectile eroded hydrodynamically; casing survived intact.";
                }
                else {
                    res.regime = "Rigid Penetration (Crater+Tunnel)";
                }
            res.shock_damage_prob_percent = 0.0;
            res.explosive_charge_survives = true;
        }
        else {
                if (res.casing_failure) {
                    // Casing already failed in the loop (e.g. J-Hook, Thermal, or Erosion Burnout)
                    res.explosive_charge_survives = false;
                    res.premature_detonation = true;
                    res.shock_damage_prob_percent = 100.0;
                }
                else {
                    // Walker-Wasley shock initiation criterion: P^2*tau >= Ec triggers
                    // detonation via the shock wave transmitted through the casing wall.
                    double criticalEnergy = std::max(1.0, proj.explosive_critical_energy);
                    res.shock_damage_prob_percent =
                        std::min(100.0, 100.0 * max_walker_wasley_product / criticalEnergy);
                    bool shockInitiates = max_walker_wasley_product >= criticalEnergy;

                        if (shockInitiates) {
                            res.explosive_charge_survives = false;
                            res.premature_detonation = true;
                            res.regime = "Shock Initiation (Walker-Wasley)";
                            res.outcome_summary =
                                "Transmitted shock exceeded the Walker-Wasley critical "
                                "initiation energy (P^2*tau >= Ec).";
                        }
                        else {
                            res.explosive_charge_survives = true;
                                if (res.erosion_occurred) {
                                    res.regime = "Hypervelocity Erosion (Walker-Anderson)";
                                }
                        }
                }
        }

    // Populate Visualization Data
    res.explosive_mass = proj.explosive_mass;
        if (res.is_kinetic_rod) {
            res.explosion_scale = 1.0;
            res.crater_wide_radius = proj.diameter * 2.0; // minimal crater
        }
        else {
            res.explosion_scale = std::max(
                5.0, (proj.yield_strength / 1e9) * 3.0); // original logic was based on casing yield
            // if we want it based on explosive mass:
            res.explosion_scale = std::max(5.0, std::min(50.0, proj.explosive_mass / 50.0));
            res.crater_wide_radius = std::min(20.0, std::max(4.5, proj.explosive_mass / 100.0));
        }
    res.crater_narrow_radius = proj.diameter / 2.0;
    res.camera_shake_magnitude = std::min(1.5, res.kinetic_energy / 1e9); // scale down energy

    // Physics-derived playback multiplier: scale so the actual simulated penetration-phase
    // duration (fast erosion vs. slow rigid drilling) maps onto a consistent, watchable window,
    // instead of a fixed arbitrary constant.
    constexpr double desiredWallClockSeconds = 6.0;
    double totalPenSimTime =
        res.penetration_frames.empty() ? dt : res.penetration_frames.back().time;
    res.time_scale_pen = (totalPenSimTime > 1.0e-9)
                             ? std::clamp(desiredWallClockSeconds / totalPenSimTime, 0.01, 5000.0)
                             : 0.02;

    return res;
}

void ImpactSimulator::printAscii3DVisualizer(const SimulationResult& r)
{
    std::cout << "\n==============================================================================="
                 "====================\n";
    std::cout << "                 ASCII 3D CROSS-SECTION VISUALIZATION: [" << r.scenario_name
              << "]\n";
    std::cout << "================================================================================="
                 "==================\n";
    std::cout << "  Velocity: " << std::fixed << std::setprecision(1) << r.velocity << " m/s (Mach "
              << r.mach_number << ") | Dyn. Press: " << std::setprecision(2)
              << (r.dynamic_pressure / 1e9)
              << " GPa | Casing Yield: " << (proj.yield_strength / 1e9) << " GPa\n";
    std::cout << "  Actual Penetration Depth: " << r.actual_penetration_depth << " m ("
              << r.actual_penetration_depth * 3.28084 << " ft) | "
              << "Shock Failure Chance: " << std::setprecision(1) << r.shock_damage_prob_percent
              << "% | "
              << "Explosive Survived: "
              << (r.explosive_charge_survives ? "YES" : "NO (SHOCK/CRUSH)") << "\n";
    std::cout << "---------------------------------------------------------------------------------"
                 "------------------\n\n";

        if (r.is_kinetic_rod) {
            std::cout << "        AIR / SPACE             |  KINETIC STRIKE ROD APPROACH (Mach "
                      << std::setprecision(1) << r.mach_number << ")\n";
            std::cout << "                                |       ||=======||    (Solid Tungsten / "
                         "Heavy Metal)\n";
            std::cout << "                                |       ||=======||    (Zero Explosive "
                         "Mass / 0 Yield Mode)\n";
            std::cout << "                                |       ||=======||    \n";
            std::cout << "    "
                         "============================+=======||=======||=========================="
                         "== [Target Surface]\n";
            std::cout << "     *    *   *  *  *  *  *  *  |       ||       ||   << HYPERVELOCITY "
                         "KINETIC EROSION >>       |\n";
            std::cout << "    *   [ KINETIC CRATERING ] * |       ||       ||   P_dyn = "
                      << std::setprecision(2) << (r.dynamic_pressure / 1e9)
                      << " GPa                  |\n";
            std::cout << "     *    *   *  *  *  *  *  *  |       ||=======||   (Penetrating via "
                         "hydrodynamic ratio)      |\n";
            std::cout << "    "
                         ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~\\.~.~.~./"
                         ".~.~.~.~.~.~.~.~.~.~.~.~.~.~. [Deep Kinetic Channel]\n";
            double target_density = target.layers.empty() ? 2500.0 : target.layers[0].density;
            std::cout << "    .   Concrete Target         |         \\     /     Max Penetration: "
                      << std::setprecision(1) << r.actual_penetration_depth << " meters    |\n";
            std::cout << "    .   (Density: " << target_density
                      << " kg/m^3) |          \\___/      (" << r.actual_penetration_depth * 3.28084
                      << " feet deep into target) |\n";
            std::cout << "    .                           |              *                         "
                         "       |\n";
            std::cout << "    "
                         ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~"
                         ".~ [Bottom of Kinetic Crater]\n";
        }
        else if (r.casing_failure || !r.explosive_charge_survives) {
            std::cout << "        AIR / SURFACE           |  PROJECTILE APPROACH (Mach "
                      << std::setprecision(1) << r.mach_number << ")\n";
            std::cout << "                                |       ||=======||    (Hardened Steel "
                         "Casing)\n";
            std::cout << "                                |       ||   *   ||    (Sensitive High "
                         "Explosive)\n";
            std::cout << "                                |       ||=======||    \n";
            std::cout << "    "
                         "============================+=======++=======++=========================="
                         "== [Target Surface]\n";
            std::cout << "     *    *   *  *  *  *  *  *  |   << IMPACT SHOCK / FAILURE ZONE >>    "
                         "     |\n";
            std::cout << "    *  ["
                      << (r.casing_failure ? "SURFACE DETONATION!" : "SHOCK FUZE FAILURE!")
                      << "] * |   (P_dyn = " << std::setprecision(1) << (r.dynamic_pressure / 1e9)
                      << " GPa, Shock Damage = " << std::setprecision(0)
                      << r.shock_damage_prob_percent << "%)     |\n";
            std::cout << "     *    *   *  *  *  *  *  *  |                                        "
                         "     |\n";
            std::cout << "    "
                         ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~"
                         ".~ [Erosion / Crater Zone]\n";
            std::cout << "    .   Concrete Target         |   Casing/Payload damaged upon impact.  "
                         "     |\n";
            double target_density2 = target.layers.empty() ? 2500.0 : target.layers[0].density;
            std::cout << "    .   (Density: " << target_density2
                      << " kg/m^3) |   Max Penetration Depth:                    |\n";
            std::cout << "    .                           |   D = " << std::setprecision(2)
                      << r.actual_penetration_depth << " m ("
                      << r.actual_penetration_depth * 3.28084 << " ft)                         |\n";
            std::cout << "    "
                         ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~"
                         ".~ [Depth Limit: "
                      << r.actual_penetration_depth << "m]\n";
            std::cout << "    .                           |   (Payload fails to reach deep "
                         "tunnels)     |\n";
        }
        else {
            std::cout << "        AIR / SURFACE           |  PROJECTILE APPROACH (Subsonic Mach "
                      << std::setprecision(1) << r.mach_number << ")\n";
            std::cout << "                                |       ||=======||    (Hardened Steel "
                         "Casing)\n";
            std::cout << "                                |       ||   *   ||    (Sensitive High "
                         "Explosive)\n";
            std::cout << "                                |       ||=======||    \n";
            std::cout << "    "
                         "============================+=======||=======||=========================="
                         "== [Target Surface]\n";
            std::cout << "                                |       ||       ||                      "
                         "     |\n";
            std::cout << "     [ RIGID PENETRATION ]      |       ||       ||   P_dyn = "
                      << std::setprecision(2) << (r.dynamic_pressure / 1e9) << " GPa           |\n";
            std::cout << "     (Casing intact & payload   |       ||   *   ||   (Shock Damage: "
                      << std::setprecision(1) << r.shock_damage_prob_percent << "%)      |\n";
            std::cout << "      survives impact shock)    |       ||=======||                      "
                         "         |\n";
            std::cout << "                                |       ||   v   ||   Rigid Depth: "
                      << std::setprecision(1) << r.rigid_penetration << " m          |\n";
            std::cout << "    "
                         ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~\\.~.~.~./"
                         ".~.~.~.~.~.~.~.~.~.~.~.~.~.~. [Drilling Deep into Rock]\n";
            std::cout << "    .   Concrete Target         |         \\     /                       "
                         "      |\n";
            double target_density3 = target.layers.empty() ? 2500.0 : target.layers[0].density;
            std::cout << "    .   (Density: " << target_density3
                      << " kg/m^3) |          \\___/  <-- Reaches " << std::setprecision(1)
                      << r.actual_penetration_depth << " meters    |\n";
            std::cout << "    .                           |              *      ("
                      << r.actual_penetration_depth * 3.28084 << " feet underground) |\n";
            std::cout << "    "
                         ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~"
                         ".~ [Deep Underground Fuze Trigger]\n";
        }
    std::cout << "================================================================================="
                 "==================\n\n";
}

void ImpactSimulator::printReport(const std::vector<SimulationResult>& results)
{
    std::cout << "\n==============================================================================="
                 "====================\n";
    std::cout << "                      PROJECTILE IMPACT & PENETRATION SIMULATION REPORT          "
                 "                  \n";
    std::cout << "================================================================================="
                 "==================\n";
    std::cout << "Projectile : " << proj.name << "\n";
    std::cout << "  - Length         : " << proj.length << " m (" << proj.length * 3.28084
              << " ft)\n";
    std::cout << "  - Total Mass     : " << proj.total_mass << " kg\n";
    std::cout << "  - Explosive Mass : " << proj.explosive_mass << " kg ("
              << (proj.explosive_mass == 0 ? "KINETIC ROD / 0 EXPLOSIVE" : "CONVENTIONAL CHARGE")
              << ")\n";
    std::cout << "  - Casing Density : " << proj.casing_density << " kg/m^3\n";
    std::cout << "  - Yield Strength : " << proj.yield_strength / 1e9 << " GPa\n";
    std::cout << "Target     : " << target.name << " (Layers: " << target.layers.size() << ")\n";
    std::cout << "---------------------------------------------------------------------------------"
                 "------------------\n";

    double total_thickness = 0.0;
    double weighted_density_sum = 0.0;
        for (const auto& layer : target.layers) {
            weighted_density_sum += layer.density * layer.thickness;
            total_thickness += layer.thickness;
        }
    double default_density = target.layers.empty() ? 2500.0 : target.layers[0].density;
    double average_density =
        (total_thickness > 0) ? (weighted_density_sum / total_thickness) : default_density;

    std::cout << "Alekseevskii-Tate Hydrodynamic Limit (Avg Target Density): P = L * sqrt(rho_p / "
                 "rho_t) = "
              << std::fixed << std::setprecision(2)
              << proj.length * std::sqrt(proj.casing_density / average_density) << " m ("
              << proj.length * std::sqrt(proj.casing_density / average_density) * 3.28084
              << " ft)\n";
    std::cout << "================================================================================="
                 "==================\n\n";

    // Print Summary Table
    std::cout << std::left << std::setw(20) << "Scenario" << std::right << std::setw(12)
              << "Velocity" << std::right << std::setw(9) << "Mach" << std::right << std::setw(14)
              << "Dyn.Press" << std::right << std::setw(12) << "Depth (m)" << std::right
              << std::setw(12) << "Shock Dmg"
              << "  " << std::left << std::setw(30) << "Regime" << std::left << std::setw(20)
              << "Outcome" << "\n";
    std::cout << std::string(130, '-') << "\n";

        for (const auto& r : results) {
            std::cout << std::left << std::setw(20) << r.scenario_name << std::right << std::setw(8)
                      << std::fixed << std::setprecision(1) << r.velocity << " m/s" << std::right
                      << std::setw(6) << std::fixed << std::setprecision(1) << r.mach_number << "x"
                      << std::right << std::setw(9) << std::fixed << std::setprecision(2)
                      << (r.dynamic_pressure / 1e9) << " GPa" << std::right << std::setw(9)
                      << std::fixed << std::setprecision(1) << r.actual_penetration_depth << " m"
                      << std::right << std::setw(10) << std::fixed << std::setprecision(0)
                      << r.shock_damage_prob_percent << "%"
                      << "  " << std::left << std::setw(30) << r.regime << std::left
                      << std::setw(20) << r.outcome_summary << "\n";
        }
    std::cout << std::string(130, '-') << "\n\n";

        // Print ASCII 3D cross sections for all simulated scenarios
        for (const auto& r : results) {
            printAscii3DVisualizer(r);
        }
}

void ImpactSimulator::generateHtml3DVisualizer(const std::vector<SimulationResult>& results,
                                               const std::string& basePath)
{
    std::ifstream tpl(basePath + "/assets/visualizer_template.html");
        if (!tpl.is_open()) {
            std::cerr << "[!] Warning: HTML Visualizer template not found at " << basePath
                      << "/assets/visualizer_template.html. Visualizer will not be generated.\n";
            return;
        }

    std::string filename = basePath + "/3d_visualizer.html";
    std::ofstream out(filename);
        if (!out.is_open()) {
            std::cerr << "Error: Could not open " << filename << " for writing.\n";
            return;
        }

    std::stringstream buffer;
    buffer << tpl.rdbuf();
    std::string html = buffer.str();

    auto escapeJSON = [](const std::string& s) {
        std::string res;
            for (char c : s) {
                if (c == '"')
                    res += "\\\"";
                else if (c == '\\')
                    res += "\\\\";
                else if (c == '\b')
                    res += "\\b";
                else if (c == '\f')
                    res += "\\f";
                else if (c == '\n')
                    res += "\\n";
                else if (c == '\r')
                    res += "\\r";
                else if (c == '\t')
                    res += "\\t";
                else
                    res += c;
            }
        return res;
    };

    // Generate scenario buttons
    std::stringstream buttons;
        for (size_t i = 0; i < results.size(); ++i) {
            buttons << "        <button onclick=\"selectScenario(" << i << ")\" id=\"btn-scenario-"
                    << i
                    << "\" class=\"scenario-btn px-4 py-1.5 rounded-full text-xs font-semibold "
                       "whitespace-nowrap "
                       "transition-all duration-200 bg-slate-800/80 hover:bg-cyan-500 "
                       "hover:text-slate-950 border border-slate-700/60\">"
                    << escapeJSON(results[i].scenario_name) << "</button>\n";
        }

    // Generate scenario data
    std::stringstream data;
        for (size_t i = 0; i < results.size(); ++i) {
            const auto& r = results[i];
            std::stringstream dropFramesJson;
            dropFramesJson << "[";
                for (size_t j = 0; j < r.drop_frames.size(); ++j) {
                    const auto& f = r.drop_frames[j];
                    dropFramesJson << "{t:" << f.time << ",y:" << f.altitude << ",v:" << f.velocity
                                   << ",m:" << f.mach
                                   << ",sb:" << (f.is_sonic_boom ? "true" : "false") << "}";
                    if (j + 1 < r.drop_frames.size())
                        dropFramesJson << ",";
                }
            dropFramesJson << "]";

            std::stringstream penFramesJson;
            penFramesJson << "[";
                for (size_t j = 0; j < r.penetration_frames.size(); ++j) {
                    const auto& f = r.penetration_frames[j];
                    penFramesJson << "{t:" << f.time << ",y:" << f.depth << ",v:" << f.velocity
                                  << ",m:" << f.mach << ",p:" << (f.dynamic_pressure / 1.0e9)
                                  << ",g:" << f.g_force << ",h:" << f.heat
                                  << ",e:" << (f.is_eroding ? "true" : "false") << ",dif:" << f.dif
                                  << ",rl:" << f.remaining_length << ",ob:" << f.obliquity_deg
                                  << "}";
                    if (j + 1 < r.penetration_frames.size())
                        penFramesJson << ",";
                }
            penFramesJson << "]";

            std::stringstream targetLayersJson;
            targetLayersJson << "[";
                for (size_t k = 0; k < target.layers.size(); ++k) {
                    const auto& lay = target.layers[k];
                    targetLayersJson << "{name:\"" << escapeJSON(lay.material_name)
                                     << "\",thickness:" << lay.thickness
                                     << ",density:" << lay.density << "}";
                    if (k + 1 < target.layers.size())
                        targetLayersJson << ",";
                }
            targetLayersJson << "]";

            data << "            { name: \"" << escapeJSON(r.scenario_name)
                 << "\", velocity: " << r.velocity << ", mach: " << r.mach_number
                 << ", energy: " << (r.kinetic_energy / 1e9)
                 << ", pressure: " << (r.dynamic_pressure / 1e9)
                 << ", yield: " << (proj.yield_strength / 1e9)
                 << ", depth: " << r.actual_penetration_depth
                 << ", prev_strike_depth: " << r.previous_strike_depth
                 << ", rigid_depth: " << r.rigid_penetration
                 << ", hydro_depth: " << r.hydro_penetration
                 << ", fail: " << (r.casing_failure ? "true" : "false")
                 << ", shock_prob: " << r.shock_damage_prob_percent
                 << ", exp_survives: " << (r.explosive_charge_survives ? "true" : "false")
                 << ", is_kinetic: " << (r.is_kinetic_rod ? "true" : "false") << ", regime: \""
                 << escapeJSON(r.regime) << "\", summary: \"" << escapeJSON(r.outcome_summary)
                 << "\""
                 << ", proj_length: " << proj.length << ", proj_diameter: " << proj.diameter
                 << ", proj_name: \"" << escapeJSON(proj.name) << "\", target_name: \""
                 << escapeJSON(target.name) << "\""
                 << ", explosive_mass: " << r.explosive_mass
                 << ", explosion_scale: " << r.explosion_scale
                 << ", crater_wide_radius: " << r.crater_wide_radius
                 << ", crater_narrow_radius: " << r.crater_narrow_radius
                 << ", camera_shake_magnitude: " << r.camera_shake_magnitude
                 << ", time_scale_pen: " << r.time_scale_pen
                 << ", erosion_occurred: " << (r.erosion_occurred ? "true" : "false")
                 << ", final_rod_length: " << r.final_rod_length
                 << ", erosion_length_lost: " << r.erosion_length_lost
                 << ", dynamic_increase_factor: " << r.dynamic_increase_factor
                 << ", bar_wave_speed: " << r.bar_wave_speed
                 << ", shock_pressure_gpa_peak: " << r.shock_pressure_gpa_peak
                 << ", shock_pulse_duration_us: " << r.shock_pulse_duration_us
                 << ", previous_strike_depth: " << r.previous_strike_depth
                 << ", cumulative_breach_depth: " << r.cumulative_breach_depth
                 << ", target_layers: " << targetLayersJson.str()
                 << ", drop_frames: " << dropFramesJson.str()
                 << ", pen_frames: " << penFramesJson.str() << " }";
            if (i + 1 < results.size())
                data << ",";
            data << "\n";
        }

    auto replaceAll = [](std::string& str, const std::string& from, const std::string& to) {
        if (from.empty())
            return;
        size_t start_pos = 0;
            while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length();
            }
    };

    replaceAll(html, "{{SCENARIO_BUTTONS}}", buttons.str());
    replaceAll(html, "/*{{SCENARIOS_DATA}}*/", data.str());

    out << html;
    out.close();

    std::cout << "\n[+] Successfully generated 3D WebGL Interactive Visualizer: " << filename
              << "\n";
    std::cout << "    -> Open " << filename
              << " in your web browser to view the 3D simulation scene!\n";
}
