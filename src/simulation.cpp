// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#include "simulation.hpp"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

ImpactSimulator::ImpactSimulator(const Projectile& p, const Target& t) : proj(p), target(t) {}

SimulationResult ImpactSimulator::simulate(const ImpactScenario& scenario)
{
    SimulationResult res;
    res.scenario_name = scenario.name;
    res.altitude_ft = scenario.altitude_ft;
    res.velocity = scenario.velocity;
    res.mach_number = scenario.velocity / SPEED_OF_SOUND;

    double squaredVelocity = std::pow(scenario.velocity, 2);

    // 1. Kinetic Energy calculation: E_k = 0.5 * m * v^2
    res.kinetic_energy = 0.5 * proj.total_mass * squaredVelocity;

    // 2. Dynamic Impact Pressure approximation: P_dyn = 0.5 * rho_t * v^2
    res.dynamic_pressure = 0.5 * target.density * squaredVelocity;

    // 3. Rigid body penetration depth into concrete/rock (Work-Energy deceleration model)
    double area = 3.14159265358979323846 * std::pow(proj.diameter / 2.0, 2);
    double cd = 1.2;     // Drag coefficient in concrete
    double rt = 100.0e6; // 100 MPa target compressive bearing strength
    res.rigid_penetration = (proj.total_mass / (2.0 * area * target.density * cd)) *
                            std::log(1.0 + (target.density * cd * squaredVelocity) / (2.0 * rt));

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
            res.actual_penetration_depth = (res.velocity > 1500.0 || proj.yield_strength == 0.0)
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
                std::min(100.0, std::max(0.0, std::pow(pressure_ratio, 1.5) * 85.0));
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

void ImpactSimulator::generateHtml3DVisualizer(const std::vector<SimulationResult>& results)
{
    std::string filename = "3d_visualizer.html";
    std::ofstream out(filename);
        if (!out.is_open()) {
            std::cerr << "Error: Could not open " << filename << " for writing.\n";
            return;
        }

    // ! /\/\/\/\/\/\/\/\/\/\/\/\
    // ! HTML
    // ! /\/\/\/\/\/\/\/\/\/\/\/\

    // ! /\/\/\/\/\/\/\/\/\/\/\/\
    // ! CSS
    // ! /\/\/\/\/\/\/\/\/\/\/\/\

    // ! /\/\/\/\/\/\/\/\/\/\/\/\
    // ! JAVA SCRIPT
    // ! /\/\/\/\/\/\/\/\/\/\/\/\

    out << R"HTML(<!DOCTYPE html>
<html lang="en" class="dark">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>3D Impact Physics & Penetration WebGL Visualizer</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <style>
        body { margin: 0; overflow: hidden; background-color: #080c14; font-family: 'Inter', system-ui, -apple-system, sans-serif; color: #f1f5f9; }
        #canvas-container { width: 100vw; height: 100vh; position: absolute; top: 0; left: 0; z-index: 1; }
        .glass-panel { background: rgba(15, 23, 42, 0.85); backdrop-filter: blur(16px); border: 1px solid rgba(255, 255, 255, 0.1); box-shadow: 0 20px 50px rgba(0, 0, 0, 0.5); }
        .slider-thumb::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 16px; height: 16px; border-radius: 50%; background: #38bdf8; cursor: pointer; box-shadow: 0 0 10px #38bdf8; }
        .slider-thumb::-moz-range-thumb { width: 16px; height: 16px; border-radius: 50%; background: #38bdf8; cursor: pointer; box-shadow: 0 0 10px #38bdf8; }
    </style>
</head>
<body class="bg-slate-950 text-slate-100 select-none">
    <div id="canvas-container"></div>
    
    <!-- Top Left Telemetry HUD -->
    <div class="absolute top-6 left-6 z-10 w-96 rounded-2xl glass-panel p-6 transition-all duration-300 hover:border-cyan-500/40">
        <div class="flex items-center justify-between border-b border-slate-700/60 pb-3 mb-4">
            <h1 class="text-xs font-black tracking-widest text-cyan-400 uppercase flex items-center gap-2">
                <span class="w-2 h-2 rounded-full bg-cyan-400 animate-pulse"></span>
                Telemetry & Impact HUD
            </h1>
            <span class="text-[10px] font-semibold px-2 py-0.5 rounded bg-slate-800 text-slate-400 border border-slate-700">WebGL 3.0</span>
        </div>
        
        <div class="space-y-2.5 text-xs">
            <div class="flex justify-between items-center"><span class="text-slate-400">Scenario Name:</span><span class="font-bold text-white truncate max-w-[180px]" id="hud-scenario">-</span></div>
            <div class="flex justify-between items-center"><span class="text-slate-400">Impact Velocity:</span><span class="font-mono font-bold text-cyan-300" id="hud-velocity">-</span></div>
            <div class="flex justify-between items-center"><span class="text-slate-400">Mach Number:</span><span class="font-mono font-semibold text-slate-200" id="hud-mach">-</span></div>
            <div class="flex justify-between items-center"><span class="text-slate-400">Dynamic Pressure:</span><span class="font-mono font-semibold text-amber-400" id="hud-pressure">-</span></div>
            <div class="flex justify-between items-center"><span class="text-slate-400">Casing Yield Limit:</span><span class="font-mono font-semibold text-slate-300" id="hud-yield">-</span></div>
            <div class="flex justify-between items-center pt-2 border-t border-slate-800"><span class="text-slate-400 font-medium">Actual Penetration:</span><span class="font-mono text-sm font-extrabold text-emerald-400" id="hud-depth">-</span></div>
            <div class="flex justify-between items-center"><span class="text-slate-400">Shock Failure Chance:</span><span class="font-mono font-bold text-rose-400" id="hud-shock">-</span></div>
        </div>

        <!-- Status Badges -->
        <div class="mt-5 pt-3 border-t border-slate-800/80 flex flex-col gap-2">
            <div id="hud-regime-badge" class="px-3 py-1.5 rounded-lg text-center text-[11px] font-black tracking-wider uppercase transition-all"></div>
            <div id="hud-exp-badge" class="px-3 py-1.5 rounded-lg text-center text-[11px] font-bold tracking-wide transition-all"></div>
        </div>
    </div>

    <!-- Top Right Camera Presets & Info -->
    <div class="absolute top-6 right-6 z-10 w-80 rounded-2xl glass-panel p-5 flex flex-col gap-4">
        <div>
            <h2 class="text-xs font-bold tracking-wider text-slate-300 uppercase mb-2">Camera Viewpoints</h2>
            <div class="grid grid-cols-2 gap-2">
                <button onclick="setView('iso')" class="px-3 py-1.5 rounded-lg bg-slate-800/80 hover:bg-cyan-500 hover:text-slate-950 font-semibold text-xs border border-slate-700 transition duration-150">Isometric</button>
                <button onclick="setView('side')" class="px-3 py-1.5 rounded-lg bg-slate-800/80 hover:bg-cyan-500 hover:text-slate-950 font-semibold text-xs border border-slate-700 transition duration-150">Side Profile</button>
                <button onclick="setView('top')" class="px-3 py-1.5 rounded-lg bg-slate-800/80 hover:bg-cyan-500 hover:text-slate-950 font-semibold text-xs border border-slate-700 transition duration-150">Top Surface</button>
                <button onclick="setView('bottom')" class="px-3 py-1.5 rounded-lg bg-slate-800/80 hover:bg-cyan-500 hover:text-slate-950 font-semibold text-xs border border-slate-700 transition duration-150">Tunnel Bottom</button>
            </div>
        </div>
        <div class="border-t border-slate-800 pt-3 text-[11px] text-slate-400 leading-relaxed" id="info-text">
            Select a scenario below or adjust parametric sliders to visualize depth and shock dynamics.
        </div>
    </div>

    <!-- Bottom Left Real-Time Parametric Controls -->
    <div class="absolute bottom-6 left-6 z-10 w-80 rounded-2xl glass-panel p-5 space-y-3">
        <h2 class="text-xs font-bold tracking-wider text-cyan-400 uppercase flex items-center justify-between">
            <span>Parametric Tuning</span>
            <span class="text-[9px] font-normal text-slate-400">Live Simulation</span>
        </h2>
        <div>
            <div class="flex justify-between text-xs mb-1"><span class="text-slate-400">Velocity (m/s):</span><span class="font-mono text-cyan-400" id="val-vel">1000</span></div>
            <input type="range" id="slider-vel" min="200" max="4500" step="50" value="1000" class="w-full h-1.5 bg-slate-700 rounded-lg appearance-none cursor-pointer slider-thumb" oninput="onSliderChange()">
        </div>
        <div>
            <div class="flex justify-between text-xs mb-1"><span class="text-slate-400">Target Density (kg/m³):</span><span class="font-mono text-cyan-400" id="val-rho">2500</span></div>
            <input type="range" id="slider-rho" min="1500" max="4000" step="100" value="2500" class="w-full h-1.5 bg-slate-700 rounded-lg appearance-none cursor-pointer slider-thumb" oninput="onSliderChange()">
        </div>
    </div>

    <!-- Bottom Center Scenario Selector Bar -->
    <div class="absolute bottom-6 left-1/2 -translate-x-1/2 z-10 rounded-full glass-panel px-4 py-2.5 flex items-center gap-2 max-w-[50vw] overflow-x-auto shadow-2xl border-cyan-500/20">
)HTML";

        for (size_t i = 0; i < results.size(); ++i) {
            out << "        <button onclick=\"selectScenario(" << i << ")\" id=\"btn-" << i
                << "\" class=\"px-4 py-1.5 rounded-full text-xs font-semibold whitespace-nowrap "
                   "transition-all duration-200 bg-slate-800/80 hover:bg-cyan-500 "
                   "hover:text-slate-950 border border-slate-700/60\">"
                << results[i].scenario_name << "</button>\n";
        }

    out << R"HTML(    </div>

    <!-- Three.js and OrbitControls -->
    <script src="https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/three@0.128.0/examples/js/controls/OrbitControls.js"></script>
    <script>
        const scenarios = [
)HTML";

        for (size_t i = 0; i < results.size(); ++i) {
            const auto& r = results[i];
            out << "            { name: \"" << r.scenario_name << "\", velocity: " << r.velocity
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
                << r.regime << "\", summary: \"" << r.outcome_summary << "\" }";
            if (i + 1 < results.size())
                out << ",";
            out << "\n";
        }

    out << R"HTML(        ];

        let scene, camera, renderer, controls;
        let targetBlock, projectile, shockwave, crater, rulerGroup;
        let currentData = scenarios[0];
        
        // Dynamically injected C++ variables
        const projName = ")HTML"
        << proj.name << R"HTML(";
        const projLength = )HTML"
        << proj.length << R"HTML(;
        const projDiameter = )HTML"
        << proj.diameter << R"HTML(;
        const projMass = )HTML"
        << proj.total_mass << R"HTML(;
        const projExplosiveMass = )HTML"
        << proj.explosive_mass << R"HTML(;
        const projCasingDensity = )HTML"
        << proj.casing_density << R"HTML(;
        const projYieldStrength = )HTML"
        << proj.yield_strength << R"HTML(;
        
        const targetName = ")HTML"
        << target.name << R"HTML(";
        const targetDensity = )HTML"
        << target.density << R"HTML(;
        const targetBearingStrength = )HTML"
        << 100.0e6 << R"HTML(; // 100 MPa
        const dragCoefficient = )HTML"
        << 1.2 << R"HTML(;
        const speedOfSound = )HTML"
        << SPEED_OF_SOUND << R"HTML(;

        function init() {
            const container = document.getElementById('canvas-container');
            scene = new THREE.Scene();
            scene.background = new THREE.Color(0x080c14);
            scene.fog = new THREE.FogExp2(0x080c14, 0.012);

            camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 0.1, 1500);
            camera.position.set(35, 20, 45);

            renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
            renderer.setSize(window.innerWidth, window.innerHeight);
            renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
            renderer.shadowMap.enabled = true;
            container.appendChild(renderer.domElement);

            controls = new THREE.OrbitControls(camera, renderer.domElement);
            controls.enableDamping = true;
            controls.dampingFactor = 0.05;
            controls.target.set(0, -15, 0);

            // Lighting
            const ambientLight = new THREE.AmbientLight(0xffffff, 0.45);
            scene.add(ambientLight);

            const dirLight = new THREE.DirectionalLight(0xffffff, 0.9);
            dirLight.position.set(30, 60, 30);
            dirLight.castShadow = true;
            scene.add(dirLight);

            const pointLight = new THREE.PointLight(0x38bdf8, 2, 80);
            pointLight.position.set(-15, 20, -15);
            scene.add(pointLight);

            // Ground Grid
            const gridHelper = new THREE.GridHelper(100, 50, 0x0ea5e9, 0x1e293b);
            gridHelper.position.y = 0;
            scene.add(gridHelper);

            // Target Concrete Block (Translucent Cube)
            const blockGeom = new THREE.BoxGeometry(24, 70, 24);
            const blockMat = new THREE.MeshPhysicalMaterial({
                color: 0x334155,
                transparent: true,
                opacity: 0.28,
                roughness: 0.6,
                metalness: 0.2,
                depthWrite: false
            });
            targetBlock = new THREE.Mesh(blockGeom, blockMat);
            targetBlock.position.set(0, -35, 0);
            scene.add(targetBlock);

            const edges = new THREE.EdgesGeometry(blockGeom);
            const line = new THREE.LineSegments(edges, new THREE.LineBasicMaterial({ color: 0x475569, linewidth: 1.5 }));
            targetBlock.add(line);

            // Projectile
            const projGeom = new THREE.CylinderGeometry(projDiameter/2, projDiameter/2, projLength, 32);
            const projMat = new THREE.MeshStandardMaterial({ color: 0xf8fafc, metalness: 0.9, roughness: 0.15 });
            projectile = new THREE.Mesh(projGeom, projMat);
            scene.add(projectile);

            // Crater / Tunnel Channel
            const craterGeom = new THREE.CylinderGeometry(1.0, 0.5, 10, 32);
            const craterMat = new THREE.MeshBasicMaterial({ color: 0x06b6d4, wireframe: true, transparent: true, opacity: 0.75 });
            crater = new THREE.Mesh(craterGeom, craterMat);
            scene.add(crater);

            // Shockwave Sphere
            const shockGeom = new THREE.SphereGeometry(3.5, 32, 32);
            const shockMat = new THREE.MeshBasicMaterial({ color: 0xef4444, transparent: true, opacity: 0.45, wireframe: true });
            shockwave = new THREE.Mesh(shockGeom, shockMat);
            shockwave.position.set(0, 0, 0);
            scene.add(shockwave);

            // 3D Depth Measurement Ruler
            rulerGroup = new THREE.Group();
            const rulerMat = new THREE.LineBasicMaterial({ color: 0x38bdf8 });
            for (let depthY = 0; depthY <= 80; depthY += 10) {
                const tickGeom = new THREE.BufferGeometry().setFromPoints([
                    new THREE.Vector3(12.5, -depthY, 12.5),
                    new THREE.Vector3(14.5, -depthY, 12.5)
                ]);
                const tickLine = new THREE.Line(tickGeom, rulerMat);
                rulerGroup.add(tickLine);
            }
            scene.add(rulerGroup);

            // Set dynamic defaults
            document.title = `${projName} vs ${targetName} - 3D Simulation Visualizer`;
            
            const rhoSlider = document.getElementById('slider-rho');
            if (targetDensity > rhoSlider.max) rhoSlider.max = targetDensity * 1.5;
            if (targetDensity < rhoSlider.min) rhoSlider.min = targetDensity * 0.5;
            rhoSlider.value = targetDensity;
            
            document.getElementById('val-rho').innerText = Math.round(targetDensity);

            window.addEventListener('resize', onWindowResize);
            selectScenario(0);
            animate();
        }

        function setView(mode) {
            if (mode === 'iso') { camera.position.set(35, 20, 45); controls.target.set(0, -15, 0); }
            else if (mode === 'side') { camera.position.set(60, -15, 0); controls.target.set(0, -15, 0); }
            else if (mode === 'top') { camera.position.set(0, 70, 1); controls.target.set(0, 0, 0); }
            else if (mode === 'bottom') { camera.position.set(20, -55, 25); controls.target.set(0, -35, 0); }
        }

        function updateVisuals(data) {
            currentData = data;
            document.getElementById('hud-scenario').innerText = data.name;
            document.getElementById('hud-velocity').innerText = data.velocity.toFixed(1) + ' m/s';
            document.getElementById('hud-mach').innerText = 'Mach ' + data.mach.toFixed(1);
            document.getElementById('hud-pressure').innerText = data.pressure.toFixed(2) + ' GPa';
            document.getElementById('hud-yield').innerText = data.yield.toFixed(2) + ' GPa';
            document.getElementById('hud-depth').innerText = data.depth.toFixed(2) + ' m (' + (data.depth * 3.28084).toFixed(1) + ' ft)';
            document.getElementById('hud-shock').innerText = data.shock_prob.toFixed(1) + '%';
            
            const regimeBadge = document.getElementById('hud-regime-badge');
            const expBadge = document.getElementById('hud-exp-badge');

            // Dynamic target block scaling
            const blockHeight = Math.max(30, data.depth * 1.35);
            targetBlock.scale.set(1, blockHeight / 70.0, 1);
            targetBlock.position.set(0, -blockHeight / 2, 0);

            if (data.is_kinetic) {
                regimeBadge.innerText = 'HYPERVELOCITY KINETIC ROD MODE';
                regimeBadge.className = 'px-3 py-1.5 rounded-lg text-center text-[11px] font-black tracking-wider uppercase bg-purple-500/20 text-purple-400 border border-purple-500/50 shadow-lg shadow-purple-500/10';
                expBadge.innerText = 'PAYLOAD: ZERO EXPLOSIVE MASS (SOLID ROD)';
                expBadge.className = 'px-3 py-1.5 rounded-lg text-center text-[11px] font-bold tracking-wide bg-slate-800/80 text-slate-300 border border-slate-700';

                projectile.position.set(0, -data.depth + (projLength/2), 0);
                projectile.scale.set(1, 1, 1);
                projectile.material.color.setHex(0xc084fc); // Neon purple
                projectile.material.emissive = new THREE.Color(0x581c87);

                crater.visible = true;
                crater.scale.set(1.2, data.depth / 10.0, 1.2);
                crater.position.set(0, -data.depth / 2, 0);
                crater.material.color.setHex(0xa855f7);

                shockwave.visible = true;
                shockwave.material.color.setHex(0xa855f7);
                shockwave.scale.set(2.2, 1.2, 2.2);

                document.getElementById('info-text').innerHTML = 
                    `<span class="text-purple-400 font-bold">Kinetic Orbital Bombardment Regime</span><br>` +
                    `Solid tungsten penetrator moving at hypervelocity generates hydrodynamic plasma erosion without explosive detonation. Depth reached: <span class="text-white font-mono">${data.depth.toFixed(1)}m</span>.`;
            } else if (data.fail || !data.exp_survives) {
                regimeBadge.innerText = data.fail ? 'CASING SHATTER / SURFACE DETONATION' : 'IMPACT SHOCK FUZE FAILURE';
                regimeBadge.className = 'px-3 py-1.5 rounded-lg text-center text-[11px] font-black tracking-wider uppercase bg-rose-500/20 text-rose-400 border border-rose-500/50 shadow-lg shadow-rose-500/10';
                expBadge.innerText = data.fail ? 'PAYLOAD DESTROYED BY STRUCTURAL CRUSH' : `EXPLOSIVE CHARGE FAILED (SHOCK: ${data.shock_prob.toFixed(0)}%)`;
                expBadge.className = 'px-3 py-1.5 rounded-lg text-center text-[11px] font-bold tracking-wide bg-rose-950/60 text-rose-300 border border-rose-800/60';

                projectile.position.set(0, -data.depth / 2, 0);
                projectile.scale.set(1.8, 0.4, 1.8); // Crushed
                projectile.material.color.setHex(0xf87171);
                projectile.material.emissive = new THREE.Color(0x7f1d1d);

                crater.visible = true;
                crater.scale.set(1.5, data.depth / 10.0, 1.5);
                crater.position.set(0, -data.depth / 2, 0);
                crater.material.color.setHex(0xef4444);

                shockwave.visible = true;
                shockwave.material.color.setHex(0xef4444);
                shockwave.scale.set(1.8, 0.8, 1.8);

                document.getElementById('info-text').innerHTML = 
                    `<span class="text-rose-400 font-bold">Structural / Shock Failure</span><br>` +
                    `Dynamic impact pressure (${data.pressure.toFixed(2)} GPa) or extreme G-deceleration shock wave fractures the bomb casing or fuze payload. Penetration halted at <span class="text-white font-mono">${data.depth.toFixed(1)}m</span>.`;
            } else {
                regimeBadge.innerText = 'RIGID DRILL PENETRATION (SUCCESS)';
                regimeBadge.className = 'px-3 py-1.5 rounded-lg text-center text-[11px] font-black tracking-wider uppercase bg-emerald-500/20 text-emerald-400 border border-emerald-500/50 shadow-lg shadow-emerald-500/10';
                expBadge.innerText = `PAYLOAD SURVIVED (SHOCK RISK: ${data.shock_prob.toFixed(1)}%)`;
                expBadge.className = 'px-3 py-1.5 rounded-lg text-center text-[11px] font-bold tracking-wide bg-emerald-950/60 text-emerald-300 border border-emerald-800/60';

                projectile.position.set(0, -data.depth + (projLength/2), 0);
                projectile.scale.set(1, 1, 1);
                projectile.material.color.setHex(0x38bdf8);
                projectile.material.emissive = new THREE.Color(0x0284c7);

                crater.visible = true;
                crater.scale.set(0.8, data.depth / 10.0, 0.8);
                crater.position.set(0, -data.depth / 2, 0);
                crater.material.color.setHex(0x06b6d4);

                shockwave.visible = false;

                document.getElementById('info-text').innerHTML = 
                    `<span class="text-emerald-400 font-bold">Deep Underground Detonation</span><br>` +
                    `Casing withstands impact pressure (< 2.0 GPa) and payload survives deceleration shock (< 50% risk). Drills cleanly down to <span class="text-white font-mono">${data.depth.toFixed(1)}m</span> before detonating.`;
            }
        }

        function selectScenario(index) {
            const data = scenarios[index];
            document.querySelectorAll('button[id^="btn-"]').forEach((btn, i) => {
                if (i === index) {
                    btn.className = "px-4 py-1.5 rounded-full text-xs font-bold whitespace-nowrap transition-all duration-200 bg-cyan-400 text-slate-950 shadow-lg shadow-cyan-400/30 scale-105";
                } else {
                    btn.className = "px-4 py-1.5 rounded-full text-xs font-semibold whitespace-nowrap transition-all duration-200 bg-slate-800/80 hover:bg-cyan-500/20 hover:text-cyan-300 text-slate-300 border border-slate-700/60";
                }
            });
            const velSlider = document.getElementById('slider-vel');
            if (data.velocity > velSlider.max) velSlider.max = data.velocity * 1.5;
            if (data.velocity < velSlider.min) velSlider.min = data.velocity * 0.5;
            velSlider.value = data.velocity;
            
            document.getElementById('val-vel').innerText = Math.round(data.velocity);
            updateVisuals(data);
        }

        function onSliderChange() {
            const vel = parseFloat(document.getElementById('slider-vel').value);
            const rho = parseFloat(document.getElementById('slider-rho').value);
            document.getElementById('val-vel').innerText = Math.round(vel);
            document.getElementById('val-rho').innerText = Math.round(rho);

            const mach = vel / speedOfSound;
            const energy = 0.5 * projMass * vel * vel;
            const pressure = 0.5 * rho * vel * vel;

            const yieldStr = projYieldStrength;
            const isKinetic = (projExplosiveMass === 0 || projYieldStrength === 0);

            const area = Math.PI * Math.pow(projDiameter / 2.0, 2);
            const cd = dragCoefficient;
            const rt = targetBearingStrength;
            const rigidDepth = (projMass / (2.0 * area * rho * cd)) * Math.log(1.0 + (rho * cd * vel * vel) / (2.0 * rt));
            const hydroDepth = projLength * Math.sqrt(projCasingDensity / rho);

            let fail = false, shockProb = 0.0, expSurvives = true, depth = rigidDepth, regime = "", summary = "";
            if (isKinetic) {
                fail = (yieldStr > 0 && pressure > yieldStr);
                depth = (vel > 1500.0 || yieldStr === 0) ? hydroDepth : rigidDepth;
                regime = "Hypervelocity Kinetic Rod Penetration";
                summary = "Hydrodynamic erosion; deep kinetic cratering without explosives";
            } else if (pressure > yieldStr) {
                fail = true;
                expSurvives = false;
                shockProb = 100.0;
                depth = hydroDepth;
                regime = "Hydrodynamic / Hypervelocity";
                summary = "Casing crushes/shatters; surface detonation";
            } else {
                fail = false;
                const pressureRatio = pressure / yieldStr;
                shockProb = Math.min(100.0, Math.max(0.0, Math.pow(pressureRatio, 1.5) * 85.0));
                expSurvives = (shockProb < 50.0);
                depth = rigidDepth;
                regime = expSurvives ? "Rigid Body Penetration" : "Rigid Body (Shock Failure)";
                summary = expSurvives ? "Casing intact; smart-fuze detonates deep underground" : "Casing intact; shock damages explosive payload";
            }

            updateVisuals({
                name: "Custom Live Parametric Tuning",
                velocity: vel,
                mach: mach,
                energy: energy / 1e9,
                pressure: pressure / 1e9,
                yield: projYieldStrength / 1e9,
                depth: depth,
                rigid_depth: rigidDepth,
                hydro_depth: hydroDepth,
                fail: fail,
                shock_prob: shockProb,
                exp_survives: expSurvives,
                is_kinetic: isKinetic,
                regime: regime,
                summary: summary
            });
            document.querySelectorAll('button[id^="btn-"]').forEach(btn => {
                btn.className = "px-4 py-1.5 rounded-full text-xs font-semibold whitespace-nowrap transition-all duration-200 bg-slate-800/80 hover:bg-cyan-500/20 hover:text-cyan-300 text-slate-300 border border-slate-700/60";
            });
        }

        function onWindowResize() {
            camera.aspect = window.innerWidth / window.innerHeight;
            camera.updateProjectionMatrix();
            renderer.setSize(window.innerWidth, window.innerHeight);
        }

        function animate() {
            requestAnimationFrame(animate);
            if (shockwave.visible) {
                shockwave.rotation.y += 0.015;
                shockwave.rotation.x += 0.008;
            }
            controls.update();
            renderer.render(scene, camera);
        }

        window.onload = init;
    </script>
</body>
</html>
)HTML";
    out.close();
    std::cout << "\n[+] Successfully generated 3D WebGL Interactive Visualizer: " << filename
              << "\n";
    std::cout << "    -> Open " << filename
              << " in your web browser to view the 3D simulation scene!\n";
}
