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

    // To achieve maximum accuracy for the rigid body penetration model, we calculate
    // the "Nose Performance Coefficient" (N) based on the Caliber-Radius-Head (CRH) geometry.
    // For a deep penetrator like the GBU-57 MOP, the CRH is typically around 3.0 to 4.0.
    double Caliber_Radius_Head = 3.0; // Caliber-Radius-Head

    // The exact geometric formula for the effective drag coefficient (N) of an ogive nose
    // penetrating a solid target (derived from the Forrestal equation):
    double dragCoefficient =
        (8.0 * Caliber_Radius_Head - 1.0) / (24.0 * std::pow(Caliber_Radius_Head, 2));

    double squaredVelocity = std::pow(scenario.velocity, 2);

    // 1. Kinetic Energy calculation: E_k = 0.5 * m * v^2
    res.kinetic_energy = 0.5 * proj.total_mass * squaredVelocity;

    // 2. Dynamic Impact Pressure approximation: P_dyn = 0.5 * rho_t * v^2
    res.dynamic_pressure = 0.5 * target.density * squaredVelocity;

    // 3. Rigid body penetration depth into concrete/rock (Work-Energy deceleration model
    double area = cons.PI * std::pow(proj.diameter / 2.0, 2);

    res.rigid_penetration = (proj.total_mass / (2.0 * area * target.density * dragCoefficient)) *
                            std::log(1.0 + (target.density * dragCoefficient * squaredVelocity) /
                                               (2.0 * target.compressiveStrength));

    // 4. Alekseevskii-Tate Hydrodynamic Limit Equation: P = L * sqrt(rho_p / rho_t)
    res.hydro_penetration = proj.length * std::sqrt(proj.casing_density / target.density);

    // 5. Kinetic rod check and structural integrity evaluation
    res.is_kinetic_rod = (proj.explosive_mass == 0.0 || proj.yield_strength == 0.0);

        if (res.is_kinetic_rod) {
            res.casing_failure =
                (proj.yield_strength > 0.0 && res.dynamic_pressure > proj.yield_strength);
            res.premature_detonation = false;     // No explosive mass to detonate prematurely
            res.explosive_charge_survives = true; // N/A for pure kinetic weapon
            res.shock_damage_prob_percent = 0.0;
            res.regime = "Hypervelocity Kinetic Rod Penetration";
            res.outcome_summary = "Hydrodynamic erosion; deep kinetic cratering without explosives";
            res.actual_penetration_depth = (res.velocity > cons.hypervelocityThreshold || proj.yield_strength == 0.0)
                                               ? res.hydro_penetration
                                               : res.rigid_penetration;
        }
        else if (res.dynamic_pressure > proj.yield_strength) {
            res.casing_failure = true;
            res.premature_detonation = true;
            res.explosive_charge_survives = false;
            res.shock_damage_prob_percent = 100.0;
            res.regime = "Hydrodynamic / Hypervelocity";
            res.outcome_summary = "Casing crushes/shatters; surface detonation";
            res.actual_penetration_depth = res.hydro_penetration;
        }
        else {
            res.casing_failure = false;
            double pressure_ratio = res.dynamic_pressure / proj.yield_strength;
            res.shock_damage_prob_percent =
                std::min(100.0, std::max(0.0, std::pow(pressure_ratio, cons.shockDamageExponent) * cons.shockDamageMultiplier));
            res.explosive_charge_survives = (res.shock_damage_prob_percent < 50.0);
                if (!res.explosive_charge_survives) {
                    res.premature_detonation = true;
                    res.regime = "Rigid Body (Shock Failure)";
                    res.outcome_summary = "Casing intact; shock damages explosive payload";
                }
                else {
                    res.premature_detonation = false;
                    res.regime = "Rigid Body Penetration";
                    res.outcome_summary = "Casing intact; smart-fuze detonates deep underground";
                }
            res.actual_penetration_depth = res.rigid_penetration;
        }

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
            std::cout << "    .   Concrete Target         |         \\     /     Max Penetration: "
                      << std::setprecision(1) << r.actual_penetration_depth << " meters    |\n";
            std::cout << "    .   (Density: " << target.density
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
            std::cout << "    .   (Density: " << target.density
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
            std::cout << "    .   (Density: " << target.density
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
    std::cout << "Target     : " << target.name << " (Density: " << target.density << " kg/m^3)\n";
    std::cout << "---------------------------------------------------------------------------------"
                 "------------------\n";
    std::cout << "Alekseevskii-Tate Hydrodynamic Limit: P = L * sqrt(rho_p / rho_t) = "
              << std::fixed << std::setprecision(2)
              << proj.length * std::sqrt(proj.casing_density / target.density) << " m ("
              << proj.length * std::sqrt(proj.casing_density / target.density) * 3.28084
              << " ft)\n";
    std::cout << "================================================================================="
                 "==================\n\n";

    // Print Summary Table
    std::cout << std::left << std::setw(20) << "Scenario" << std::right << std::setw(10)
              << "Velocity" << std::right << std::setw(7) << "Mach" << std::right << std::setw(11)
              << "Dyn.Press" << std::right << std::setw(11) << "Depth (m)" << std::right
              << std::setw(11) << "Shock Dmg"
              << "  " << std::left << std::setw(22) << "Regime" << std::left << std::setw(20)
              << "Outcome" << "\n";
    std::cout << std::string(112, '-') << "\n";

        for (const auto& r : results) {
            std::cout << std::left << std::setw(20) << r.scenario_name << std::right << std::setw(8)
                      << std::fixed << std::setprecision(1) << r.velocity << "m/s" << std::right
                      << std::setw(6) << std::fixed << std::setprecision(1) << r.mach_number << "x"
                      << std::right << std::setw(9) << std::fixed << std::setprecision(2)
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

void ImpactSimulator::generateHtml3DVisualizer(const std::vector<SimulationResult>& results, const std::string& basePath)
{
    std::string filename = basePath + "/3d_visualizer.html";
    std::ofstream out(filename);
        if (!out.is_open()) {
            std::cerr << "Error: Could not open " << filename << " for writing.\n";
            return;
        }

    std::ifstream tpl(basePath + "/assets/visualizer_template.html");
        if (!tpl.is_open()) {
            std::cerr << "Error: Could not open template file " << basePath << "/assets/visualizer_template.html\n";
            return;
        }

    std::stringstream buffer;
    buffer << tpl.rdbuf();
    std::string html = buffer.str();

    auto escapeJSON = [](const std::string& s) {
        std::string res;
        for (char c : s) {
            if (c == '"') res += "\\\"";
            else if (c == '\\') res += "\\\\";
            else if (c == '\b') res += "\\b";
            else if (c == '\f') res += "\\f";
            else if (c == '\n') res += "\\n";
            else if (c == '\r') res += "\\r";
            else if (c == '\t') res += "\\t";
            else res += c;
        }
        return res;
    };

    // Generate scenario buttons
    std::stringstream buttons;
        for (size_t i = 0; i < results.size(); ++i) {
            buttons
                << "        <button onclick=\"selectScenario(" << i << ")\" id=\"btn-" << i
                << "\" class=\"px-4 py-1.5 rounded-full text-xs font-semibold whitespace-nowrap "
                   "transition-all duration-200 bg-slate-800/80 hover:bg-cyan-500 "
                   "hover:text-slate-950 border border-slate-700/60\">"
                << escapeJSON(results[i].scenario_name) << "</button>\n";
        }

    // Generate scenario data
    std::stringstream data;
        for (size_t i = 0; i < results.size(); ++i) {
            const auto& r = results[i];
            data << "            { name: \"" << escapeJSON(r.scenario_name) << "\", velocity: " << r.velocity
                 << ", mach: " << r.mach_number << ", energy: " << (r.kinetic_energy / 1e9)
                 << ", pressure: " << (r.dynamic_pressure / 1e9)
                 << ", yield: " << (proj.yield_strength / 1e9)
                 << ", depth: " << r.actual_penetration_depth
                 << ", rigid_depth: " << r.rigid_penetration
                 << ", hydro_depth: " << r.hydro_penetration
                 << ", fail: " << (r.casing_failure ? "true" : "false")
                 << ", shock_prob: " << r.shock_damage_prob_percent
                 << ", exp_survives: " << (r.explosive_charge_survives ? "true" : "false")
                 << ", is_kinetic: " << (r.is_kinetic_rod ? "true" : "false") << ", regime: \""
                 << escapeJSON(r.regime) << "\", summary: \"" << escapeJSON(r.outcome_summary) << "\" }";
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
