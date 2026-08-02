// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#include "simulation.hpp"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

ImpactSimulator::ImpactSimulator(const Projectile& p, const Target& t, const PhysicsConstants& c)
    : proj(p), target(t), cons(c)
{
}

SimulationResult ImpactSimulator::simulate(const ImpactScenario& scenario)
{
    SimulationResult res;
    res.scenario_name = scenario.name;
    res.altitude_ft = scenario.altitude_ft;
    res.velocity = scenario.velocity;
    res.mach_number = scenario.velocity / cons.SPEED_OF_SOUND;

    double area = cons.PI * std::pow(proj.diameter / 2.0, 2);

    double CRH = 0.0;
        if (proj.diameter > 0.0) {
            CRH = proj.curvature_noseReduce / proj.diameter;
        }

    // Calculate Nose Performance Coefficient (N)
    double Caliber_Radius_Head = (CRH > 0.0) ? CRH : 3.0;
    double dragCoefficient =
        (8.0 * Caliber_Radius_Head - 1.0) / (24.0 * std::pow(Caliber_Radius_Head, 2));

    // ! new down here

    AirLayers eachAirLayer;
    AltitudeDensityPoint airLayerData;

    double maxAltitude_ft = eachAirLayer.eachLayer.size() - 1;
    double dropAltitude = scenario.altitude_ft;

    double current_altitude;

    double findAirDensityByAltitude(const std::vector<double>& eachAirLayer.eachLayer) {
        std::find_if(eachLayer.rbegin(), eachLayer.rend()),
        [current_altitude](const current_density& p)

        { return p.Altitude == targetAltitude }
    };

    size_t current_air_layer_idx = 0;

    // For now!

        // Time Integration Loop
        while (res.casing_failure == false && current_altitude > 0.0 &&
               current_altitude < dropAltitude) {
                // Advance layer if we've pierced the current one
                while (current_air_layer_idx < Max_Altitude_Array.size() &&
                       current_altitude >= Max_Altitude_Array[current_air_layer_idx]) {
                    current_air_layer_idx++;
                }

            double velocity = std::sqrt(
                ((2.0 * proj.total_mass * 9.81) / (*area * dragCoefficient)) *
                (1.0 -
                 std::exp(-(air.levels * area * dragCoefficient * (scenario.altitude_ft * 0.3048)) /
                          proj.total_mass)));

            // ! new up there

            double current_velocity = scenario.velocity;
            double current_mass = proj.total_mass;
            double current_depth = 0.0;

            // Obliquity and AoA
            double obliquity_radians = scenario.obliquity_angle * cons.PI / 180.0;
            double angleOfAttack_radians = scenario.angle_of_attack * cons.PI / 180.0;

            double dt = 1e-5; // 10 microseconds
            double t = 0.0;
            double current_temperature = 300.0; // Kelvin

            res.casing_failure = false;
            res.premature_detonation = false;
            res.explosive_charge_survives = true;
            res.shock_damage_prob_percent = 0.0;
            res.regime = "Time-Integrated Penetration";
            res.outcome_summary = "Intact";

            double max_dynamic_pressure = 0.0;

            // Convert target layers to fullDepth depths
            std::vector<double> layer_bottom_depths;
            double fullDepth = 0.0;
                for (const auto& layer : target.layers) {
                    fullDepth += layer.thickness;
                    layer_bottom_depths.push_back(fullDepth);
                }

            size_t current_layer_idx = 0;

                // Time Integration Loop
                while (current_velocity > 0.0 && res.casing_failure == false &&
                       current_depth < fullDepth) {
                        // Advance layer if we've pierced the current one
                        while (current_layer_idx < layer_bottom_depths.size() &&
                               current_depth >= layer_bottom_depths[current_layer_idx]) {
                            current_layer_idx++;
                        }

                        if (current_layer_idx >= target.layers.size()) {
                            res.regime = "Target Perforated";
                            res.outcome_summary =
                                "Projectile completely pierced all target layers.";
                            break;
                        }

                    const auto& layer = target.layers[current_layer_idx];

                    double squaredVelocity = current_velocity * current_velocity;

                    // Dynamic Pressure
                    double dynamic_pressure = 0.5 * layer.density * squaredVelocity;
                        if (dynamic_pressure > max_dynamic_pressure) {
                            max_dynamic_pressure = dynamic_pressure;
                        }

                    // Bending Moment calculation (Asymmetric force)
                    double asymmetric_force = dynamic_pressure * area *
                                              std::sin(obliquity_radians + angleOfAttack_radians);
                    double bending_moment =
                        asymmetric_force * (proj.length / 2.0); // Simplified load

                    // Stress = M * y / I
                    double max_bending_stress = 0.0;
                        if (proj.area_moment_inertia > 0) {
                            max_bending_stress =
                                (bending_moment * (proj.diameter / 2.0)) / proj.area_moment_inertia;
                        }

                        // Check structural failure
                        if (proj.yield_strength > 0.0 && max_bending_stress > proj.yield_strength) {
                            res.casing_failure = true;
                            res.regime = "Structural Failure (J-Hook/Snap)";
                            res.outcome_summary = "Bending moments exceeded casing yield strength.";
                            break;
                        }

                    // Deceleration force (incorporating rebar)
                    // F = area * (strength + rebar_strength * rebar_frac) + drag * density * area *
                    // v^2
                    double effective_strength =
                        layer.compressiveStrength +
                        (layer.rebar_yield_strength * layer.rebar_volume_fraction);
                    double deceleration_force =
                        (area * effective_strength) +
                        (dragCoefficient * layer.density * area * squaredVelocity);

                    // Acceleration
                    double safe_mass = (current_mass > 0.001) ? current_mass : 0.001;
                    double acceleration = -deceleration_force / safe_mass;

                    // Kinematics update
                    current_velocity += acceleration * dt;
                    current_depth += current_velocity * dt * std::cos(obliquity_radians);

                    // Thermal Ablation (Simplified Frictional Heating)
                    // Energy converted to heat = Force * distance * friction_factor
                    double friction_factor = cons.frictionFactor;
                    double heat_energy =
                        (deceleration_force * friction_factor) * (current_velocity * dt);
                    double temp_increase = heat_energy / (safe_mass * proj.specific_heat);
                    current_temperature += temp_increase;

                        // If we reach melting point, mass is lost to heat of fusion
                        if (current_temperature > proj.melting_point) {
                            double excess_temp = current_temperature - proj.melting_point;
                            double excess_heat = excess_temp * safe_mass * proj.specific_heat;

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

                    t += dt;

                    // Failsafe for infinite loop (e.g. 10 seconds max)
                    if (t > 10.0)
                        break;
                }

            res.actual_penetration_depth = current_depth;
            res.dynamic_pressure = max_dynamic_pressure;
            res.kinetic_energy = 0.5 * proj.total_mass * std::pow(scenario.velocity, 2);
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
            res.is_kinetic_rod = (proj.explosive_mass == 0.0 || proj.yield_strength == 0.0);

                if (res.is_kinetic_rod) {
                        if (res.casing_failure) {
                            // Already failed due to bending or thermal destruction
                        }
                        else if (max_dynamic_pressure > proj.yield_strength &&
                                 proj.yield_strength > 0) {
                            res.casing_failure = true;
                            res.regime = "Hydrodynamic Yield";
                            res.outcome_summary = "Kinetic rod crushed by pressure.";
                        }
                        else {
                            res.regime = "Hypervelocity Kinetic Rod Penetration";
                        }
                    res.shock_damage_prob_percent = 0.0;
                    res.explosive_charge_survives = true;
                }
                else {
                        if (res.casing_failure) {
                            // Casing already failed in the loop (e.g. J-Hook or Thermal)
                            res.explosive_charge_survives = false;
                            res.premature_detonation = true;
                            res.shock_damage_prob_percent = 100.0;
                        }
                        else if (max_dynamic_pressure > proj.yield_strength) {
                            res.casing_failure = true;
                            res.premature_detonation = true;
                            res.explosive_charge_survives = false;
                            res.shock_damage_prob_percent = 100.0;
                            res.regime = "Pressure Yield (Crush)";
                            res.outcome_summary = "Casing crushed by dynamic pressure.";
                        }
                        else {
                            double pressure_ratio = max_dynamic_pressure / proj.yield_strength;
                            res.shock_damage_prob_percent = std::min(
                                100.0,
                                std::max(0.0,
                                         std::pow(pressure_ratio, cons.shockDamageExponent) *
                                             cons.shockDamageMultiplier));
                            res.explosive_charge_survives = (res.shock_damage_prob_percent < 50.0);
                                if (!res.explosive_charge_survives) {
                                    res.premature_detonation = true;
                                    res.regime = "Shock Failure";
                                    res.outcome_summary =
                                        "Explosive charge detonated prematurely due to shock.";
                                }
                        }
                }

            return res;
        }

    void ImpactSimulator::printAscii3DVisualizer(const SimulationResult& r)
    {
        std::cout
            << "\n==============================================================================="
               "====================\n";
        std::cout << "                 ASCII 3D CROSS-SECTION VISUALIZATION: [" << r.scenario_name
                  << "]\n";
        std::cout
            << "================================================================================="
               "==================\n";
        std::cout << "  Velocity: " << std::fixed << std::setprecision(1) << r.velocity
                  << " m/s (Mach " << r.mach_number << ") | Dyn. Press: " << std::setprecision(2)
                  << (r.dynamic_pressure / 1e9)
                  << " GPa | Casing Yield: " << (proj.yield_strength / 1e9) << " GPa\n";
        std::cout << "  Actual Penetration Depth: " << r.actual_penetration_depth << " m ("
                  << r.actual_penetration_depth * 3.28084 << " ft) | "
                  << "Shock Failure Chance: " << std::setprecision(1) << r.shock_damage_prob_percent
                  << "% | "
                  << "Explosive Survived: "
                  << (r.explosive_charge_survives ? "YES" : "NO (SHOCK/CRUSH)") << "\n";
        std::cout
            << "---------------------------------------------------------------------------------"
               "------------------\n\n";

            if (r.is_kinetic_rod) {
                std::cout << "        AIR / SPACE             |  KINETIC STRIKE ROD APPROACH (Mach "
                          << std::setprecision(1) << r.mach_number << ")\n";
                std::cout
                    << "                                |       ||=======||    (Solid Tungsten / "
                       "Heavy Metal)\n";
                std::cout
                    << "                                |       ||=======||    (Zero Explosive "
                       "Mass / 0 Yield Mode)\n";
                std::cout << "                                |       ||=======||    \n";
                std::cout
                    << "    "
                       "============================+=======||=======||=========================="
                       "== [Target Surface]\n";
                std::cout
                    << "     *    *   *  *  *  *  *  *  |       ||       ||   << HYPERVELOCITY "
                       "KINETIC EROSION >>       |\n";
                std::cout << "    *   [ KINETIC CRATERING ] * |       ||       ||   P_dyn = "
                          << std::setprecision(2) << (r.dynamic_pressure / 1e9)
                          << " GPa                  |\n";
                std::cout
                    << "     *    *   *  *  *  *  *  *  |       ||=======||   (Penetrating via "
                       "hydrodynamic ratio)      |\n";
                std::cout << "    "
                             ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~\\.~.~.~./"
                             ".~.~.~.~.~.~.~.~.~.~.~.~.~.~. [Deep Kinetic Channel]\n";
                double target_density = target.layers.empty() ? 2500.0 : target.layers[0].density;
                std::cout
                    << "    .   Concrete Target         |         \\     /     Max Penetration: "
                    << std::setprecision(1) << r.actual_penetration_depth << " meters    |\n";
                std::cout << "    .   (Density: " << target_density
                          << " kg/m^3) |          \\___/      ("
                          << r.actual_penetration_depth * 3.28084 << " feet deep into target) |\n";
                std::cout
                    << "    .                           |              *                         "
                       "       |\n";
                std::cout
                    << "    "
                       ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~"
                       ".~ [Bottom of Kinetic Crater]\n";
            }
            else if (r.casing_failure || !r.explosive_charge_survives) {
                std::cout << "        AIR / SURFACE           |  PROJECTILE APPROACH (Mach "
                          << std::setprecision(1) << r.mach_number << ")\n";
                std::cout
                    << "                                |       ||=======||    (Hardened Steel "
                       "Casing)\n";
                std::cout
                    << "                                |       ||   *   ||    (Sensitive High "
                       "Explosive)\n";
                std::cout << "                                |       ||=======||    \n";
                std::cout
                    << "    "
                       "============================+=======++=======++=========================="
                       "== [Target Surface]\n";
                std::cout
                    << "     *    *   *  *  *  *  *  *  |   << IMPACT SHOCK / FAILURE ZONE >>    "
                       "     |\n";
                std::cout << "    *  ["
                          << (r.casing_failure ? "SURFACE DETONATION!" : "SHOCK FUZE FAILURE!")
                          << "] * |   (P_dyn = " << std::setprecision(1)
                          << (r.dynamic_pressure / 1e9)
                          << " GPa, Shock Damage = " << std::setprecision(0)
                          << r.shock_damage_prob_percent << "%)     |\n";
                std::cout
                    << "     *    *   *  *  *  *  *  *  |                                        "
                       "     |\n";
                std::cout
                    << "    "
                       ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~"
                       ".~ [Erosion / Crater Zone]\n";
                std::cout
                    << "    .   Concrete Target         |   Casing/Payload damaged upon impact.  "
                       "     |\n";
                double target_density2 = target.layers.empty() ? 2500.0 : target.layers[0].density;
                std::cout << "    .   (Density: " << target_density2
                          << " kg/m^3) |   Max Penetration Depth:                    |\n";
                std::cout << "    .                           |   D = " << std::setprecision(2)
                          << r.actual_penetration_depth << " m ("
                          << r.actual_penetration_depth * 3.28084
                          << " ft)                         |\n";
                std::cout
                    << "    "
                       ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~"
                       ".~ [Depth Limit: "
                    << r.actual_penetration_depth << "m]\n";
                std::cout << "    .                           |   (Payload fails to reach deep "
                             "tunnels)     |\n";
            }
            else {
                std::cout
                    << "        AIR / SURFACE           |  PROJECTILE APPROACH (Subsonic Mach "
                    << std::setprecision(1) << r.mach_number << ")\n";
                std::cout
                    << "                                |       ||=======||    (Hardened Steel "
                       "Casing)\n";
                std::cout
                    << "                                |       ||   *   ||    (Sensitive High "
                       "Explosive)\n";
                std::cout << "                                |       ||=======||    \n";
                std::cout
                    << "    "
                       "============================+=======||=======||=========================="
                       "== [Target Surface]\n";
                std::cout
                    << "                                |       ||       ||                      "
                       "     |\n";
                std::cout << "     [ RIGID PENETRATION ]      |       ||       ||   P_dyn = "
                          << std::setprecision(2) << (r.dynamic_pressure / 1e9)
                          << " GPa           |\n";
                std::cout << "     (Casing intact & payload   |       ||   *   ||   (Shock Damage: "
                          << std::setprecision(1) << r.shock_damage_prob_percent << "%)      |\n";
                std::cout
                    << "      survives impact shock)    |       ||=======||                      "
                       "         |\n";
                std::cout << "                                |       ||   v   ||   Rigid Depth: "
                          << std::setprecision(1) << r.rigid_penetration << " m          |\n";
                std::cout << "    "
                             ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~\\.~.~.~./"
                             ".~.~.~.~.~.~.~.~.~.~.~.~.~.~. [Drilling Deep into Rock]\n";
                std::cout
                    << "    .   Concrete Target         |         \\     /                       "
                       "      |\n";
                double target_density3 = target.layers.empty() ? 2500.0 : target.layers[0].density;
                std::cout << "    .   (Density: " << target_density3
                          << " kg/m^3) |          \\___/  <-- Reaches " << std::setprecision(1)
                          << r.actual_penetration_depth << " meters    |\n";
                std::cout << "    .                           |              *      ("
                          << r.actual_penetration_depth * 3.28084 << " feet underground) |\n";
                std::cout
                    << "    "
                       ".~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~"
                       ".~ [Deep Underground Fuze Trigger]\n";
            }
        std::cout
            << "================================================================================="
               "==================\n\n";
    }

    void ImpactSimulator::printReport(const std::vector<SimulationResult>& results)
    {
        std::cout
            << "\n==============================================================================="
               "====================\n";
        std::cout
            << "                      PROJECTILE IMPACT & PENETRATION SIMULATION REPORT          "
               "                  \n";
        std::cout
            << "================================================================================="
               "==================\n";
        std::cout << "Projectile : " << proj.name << "\n";
        std::cout << "  - Length         : " << proj.length << " m (" << proj.length * 3.28084
                  << " ft)\n";
        std::cout << "  - Total Mass     : " << proj.total_mass << " kg\n";
        std::cout << "  - Explosive Mass : " << proj.explosive_mass << " kg ("
                  << (proj.explosive_mass == 0 ? "KINETIC ROD / 0 EXPLOSIVE"
                                               : "CONVENTIONAL CHARGE")
                  << ")\n";
        std::cout << "  - Casing Density : " << proj.casing_density << " kg/m^3\n";
        std::cout << "  - Yield Strength : " << proj.yield_strength / 1e9 << " GPa\n";
        std::cout << "Target     : " << target.name << " (Layers: " << target.layers.size()
                  << ")\n";
        std::cout
            << "---------------------------------------------------------------------------------"
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

        std::cout
            << "Alekseevskii-Tate Hydrodynamic Limit (Avg Target Density): P = L * sqrt(rho_p / "
               "rho_t) = "
            << std::fixed << std::setprecision(2)
            << proj.length * std::sqrt(proj.casing_density / average_density) << " m ("
            << proj.length * std::sqrt(proj.casing_density / average_density) * 3.28084 << " ft)\n";
        std::cout
            << "================================================================================="
               "==================\n\n";

        // Print Summary Table
        std::cout << std::left << std::setw(20) << "Scenario" << std::right << std::setw(10)
                  << "Velocity" << std::right << std::setw(7) << "Mach" << std::right
                  << std::setw(11) << "Dyn.Press" << std::right << std::setw(11) << "Depth (m)"
                  << std::right << std::setw(11) << "Shock Dmg"
                  << "  " << std::left << std::setw(22) << "Regime" << std::left << std::setw(20)
                  << "Outcome" << "\n";
        std::cout << std::string(112, '-') << "\n";

            for (const auto& r : results) {
                std::cout << std::left << std::setw(20) << r.scenario_name << std::right
                          << std::setw(8) << std::fixed << std::setprecision(1) << r.velocity
                          << "m/s" << std::right << std::setw(6) << std::fixed
                          << std::setprecision(1) << r.mach_number << "x" << std::right
                          << std::setw(9) << std::fixed << std::setprecision(2)
                          << (r.dynamic_pressure / 1e9) << "GPa" << std::right << std::setw(9)
                          << std::fixed << std::setprecision(1) << r.actual_penetration_depth << "m"
                          << std::right << std::setw(9) << std::fixed << std::setprecision(0)
                          << r.shock_damage_prob_percent << "%"
                          << "  " << std::left << std::setw(22) << r.regime << std::left
                          << std::setw(20) << r.outcome_summary << "\n";
            }
        std::cout << std::string(112, '-') << "\n\n";

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
                std::cerr
                    << "[!] Warning: HTML Visualizer template not found at " << basePath
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
                buttons << "        <button onclick=\"selectScenario(" << i << ")\" id=\"btn-" << i
                        << "\" class=\"px-4 py-1.5 rounded-full text-xs font-semibold "
                           "whitespace-nowrap "
                           "transition-all duration-200 bg-slate-800/80 hover:bg-cyan-500 "
                           "hover:text-slate-950 border border-slate-700/60\">"
                        << escapeJSON(results[i].scenario_name) << "</button>\n";
            }

        // Generate scenario data
        std::stringstream data;
            for (size_t i = 0; i < results.size(); ++i) {
                const auto& r = results[i];
                data << "            { name: \"" << escapeJSON(r.scenario_name)
                     << "\", velocity: " << r.velocity << ", mach: " << r.mach_number
                     << ", energy: " << (r.kinetic_energy / 1e9)
                     << ", pressure: " << (r.dynamic_pressure / 1e9)
                     << ", yield: " << (proj.yield_strength / 1e9)
                     << ", depth: " << r.actual_penetration_depth
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
                     << escapeJSON(target.name) << "\" }";
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
        replaceAll(html, "{{SCENARIOS_DATA}}", data.str());

        out << html;
        out.close();

        std::cout << "\n[+] Successfully generated 3D WebGL Interactive Visualizer: " << filename
                  << "\n";
        std::cout << "    -> Open " << filename
                  << " in your web browser to view the 3D simulation scene!\n";
    }
