#include "simulation.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <sstream>

ImpactSimulator::ImpactSimulator(const Projectile& p, const Target& t) : proj(p), target(t) {}

SimulationResult ImpactSimulator::simulate(const ImpactScenario& scenario) {
    SimulationResult res;
    res.scenario_name = scenario.name;
    res.altitude_ft = scenario.altitude_ft;
    res.velocity = scenario.velocity;
    res.mach_number = scenario.velocity / SPEED_OF_SOUND;`

    double squaredVelocity = std::pow(scenario.velocity, 2);

    // 1. Kinetic Energy calculation: E_k = 0.5 * m * v^2
    res.kinetic_energy = 0.5 * proj.total_mass * squaredVelocity;

    // 2. Dynamic Impact Pressure approximation: P_dyn = 0.5 * rho_t * v^2
    res.dynamic_pressure = 0.5 * target.density * squaredVelocity;

    // 3. Structural integrity and detonation evaluation
    if (res.dynamic_pressure > proj.yield_strength) {
        res.casing_failure = true;
        res.premature_detonation = true;
        res.regime = "Hydrodynamic / Hypervelocity";
        res.outcome_summary = "Casing crushes/shatters; surface detonation";
    } else {
        res.casing_failure = false;
        res.premature_detonation = false;
        res.regime = "Rigid Body Penetration";
        res.outcome_summary = "Casing intact; smart-fuze detonates deep underground";
    }

    // 4. Alekseevskii-Tate Hydrodynamic Limit Equation: P = L * sqrt(rho_p / rho_t)
    res.hydro_penetration = proj.length * std::sqrt(proj.casing_density / target.density);

    return res;
}

void ImpactSimulator::printAscii3DVisualizer(const SimulationResult& r) {
    std::cout << "\n===================================================================================================\n";
    std::cout << "                 ASCII 3D CROSS-SECTION VISUALIZATION: [" << r.scenario_name << "]\n";
    std::cout << "===================================================================================================\n";
    std::cout << "  Velocity: " << std::fixed << std::setprecision(1) << r.velocity << " m/s (Mach " << r.mach_number << ") | Dyn. Press: " 
              << std::setprecision(2) << (r.dynamic_pressure / 1e9) << " GPa | Casing Yield: " << (proj.yield_strength / 1e9) << " GPa\n";
    std::cout << "---------------------------------------------------------------------------------------------------\n\n";

    if (r.casing_failure) {
        std::cout << "        AIR / SURFACE           |  PROJECTILE APPROACH (Hypervelocity Mach " << std::setprecision(1) << r.mach_number << ")\n";
        std::cout << "                                |       ||=======||    (Hardened Steel Casing)\n";
        std::cout << "                                |       ||   *   ||    (Sensitive High Explosive)\n";
        std::cout << "                                |       ||=======||    \n";
        std::cout << "    ============================+=======++=======++============================ [Target Surface]\n";
        std::cout << "     *    *   *  *  *  *  *  *  |   << HYDRODYNAMIC IMPACT WAVE >>            |\n";
        std::cout << "    *  [SURFACE DETONATION!]  * |   (P_dyn = " << std::setprecision(1) << (r.dynamic_pressure/1e9) << " GPa > Yield 2.0 GPa)       |\n";
        std::cout << "     *    *   *  *  *  *  *  *  |                                             |\n";
        std::cout << "    .~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~ [Fluid Erosion Zone]\n";
        std::cout << "    .   Concrete Target         |   Casing crushes & shatters instantly.      |\n";
        std::cout << "    .   (Density: " << target.density << " kg/m^3) |   Max Fluid Penetration Depth:              |\n";
        std::cout << "    .                           |   P = L * sqrt(rho_p / rho_t) = " << std::setprecision(2) << r.hydro_penetration << " m  |\n";
        std::cout << "    .~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~ [Depth Limit: " << r.hydro_penetration << "m]\n";
        std::cout << "    .                           |   (Payload fails to reach deep tunnels)     |\n";
    } else {
        std::cout << "        AIR / SURFACE           |  PROJECTILE APPROACH (Subsonic Mach " << std::setprecision(1) << r.mach_number << ")\n";
        std::cout << "                                |       ||=======||    (Hardened Steel Casing)\n";
        std::cout << "                                |       ||   *   ||    (Sensitive High Explosive)\n";
        std::cout << "                                |       ||=======||    \n";
        std::cout << "    ============================+=======||=======||============================ [Target Surface]\n";
        std::cout << "                                |       ||       ||                           |\n";
        std::cout << "     [ RIGID PENETRATION ]      |       ||       ||   P_dyn = " << std::setprecision(2) << (r.dynamic_pressure/1e9) << " GPa           |\n";
        std::cout << "     (Casing remains intact;    |       ||   *   ||   (Well below 2.0 GPa yield)  |\n";
        std::cout << "      smart-fuze protected)     |       ||=======||                               |\n";
        std::cout << "                                |       ||   v   ||                               |\n";
        std::cout << "    .~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~\\.~.~.~./.~.~.~.~.~.~.~.~.~.~.~.~.~.~. [Drilling Deep into Rock]\n";
        std::cout << "    .   Concrete Target         |         \\     /                             |\n";
        std::cout << "    .   (Density: " << target.density << " kg/m^3) |          \\___/  <-- Reaches 40 - 60+ meters |\n";
        std::cout << "    .                           |              *                              |\n";
        std::cout << "    .~.~.~.~.~.~.~.~.~.~.~.~.~.~+.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~ [Deep Underground Fuze Trigger]\n";
    }
    std::cout << "===================================================================================================\n\n";
}

void ImpactSimulator::printReport(const std::vector<SimulationResult>& results) {
    std::cout << "\n===================================================================================================\n";
    std::cout << "                      PROJECTILE IMPACT & PENETRATION SIMULATION REPORT                            \n";
    std::cout << "===================================================================================================\n";
    std::cout << "Projectile : " << proj.name << "\n";
    std::cout << "  - Length         : " << proj.length << " m (" << proj.length * 3.28084 << " ft)\n";
    std::cout << "  - Total Mass     : " << proj.total_mass << " kg\n";
    std::cout << "  - Casing Density : " << proj.casing_density << " kg/m^3\n";
    std::cout << "  - Yield Strength : " << proj.yield_strength / 1e9 << " GPa\n";
    std::cout << "Target     : " << target.name << " (Density: " << target.density << " kg/m^3)\n";
    std::cout << "---------------------------------------------------------------------------------------------------\n";
    std::cout << "Alekseevskii-Tate Hydrodynamic Limit: P = L * sqrt(rho_p / rho_t) = " 
              << std::fixed << std::setprecision(2) << proj.length * std::sqrt(proj.casing_density / target.density) 
              << " m (" << proj.length * std::sqrt(proj.casing_density / target.density) * 3.28084 << " ft)\n";
    std::cout << "===================================================================================================\n\n";

    // Print Summary Table
    std::cout << std::left << std::setw(22) << "Scenario"
              << std::right << std::setw(12) << "Velocity"
              << std::right << std::setw(8) << "Mach"
              << std::right << std::setw(12) << "Kin. Energy"
              << std::right << std::setw(12) << "Dyn. Press."
              << "  " << std::left << std::setw(26) << "Regime"
              << std::left << std::setw(20) << "Outcome" << "\n";
    std::cout << std::string(112, '-') << "\n";

    for (const auto& r : results) {
        std::cout << std::left << std::setw(22) << r.scenario_name
                  << std::right << std::setw(10) << std::fixed << std::setprecision(1) << r.velocity << " m/s"
                  << std::right << std::setw(7) << std::fixed << std::setprecision(1) << r.mach_number << "x"
                  << std::right << std::setw(9) << std::fixed << std::setprecision(2) << (r.kinetic_energy / 1e9) << " GJ"
                  << std::right << std::setw(9) << std::fixed << std::setprecision(2) << (r.dynamic_pressure / 1e9) << " GPa"
                  << "  " << std::left << std::setw(26) << r.regime
                  << std::left << std::setw(20) << r.outcome_summary << "\n";
    }
    std::cout << std::string(112, '-') << "\n\n";

    // Print ASCII 3D cross sections for all simulated scenarios
    for (const auto& r : results) {
        printAscii3DVisualizer(r);
    }
}

void ImpactSimulator::generateHtml3DVisualizer(const std::vector<SimulationResult>& results) {
    std::string filename = "3d_visualizer.html";
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error: Could not open " << filename << " for writing.\n";
        return;
    }

    out << R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>3D Impact Physics Simulator - Three.js WebGL Visualizer</title>
    <style>
        body { margin: 0; overflow: hidden; background-color: #0d0f12; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; color: #e0e6ed; }
        #canvas-container { width: 100vw; height: 100vh; position: absolute; top: 0; left: 0; z-index: 1; }
        #hud { position: absolute; top: 20px; left: 20px; z-index: 10; background: rgba(18, 22, 28, 0.85); backdrop-filter: blur(12px); border: 1px solid rgba(255,255,255,0.15); border-radius: 12px; padding: 24px; width: 380px; box-shadow: 0 8px 32px 0 rgba(0,0,0,0.37); }
        h1 { font-size: 18px; margin: 0 0 16px 0; color: #4facfe; text-transform: uppercase; letter-spacing: 1.5px; border-bottom: 1px solid rgba(255,255,255,0.1); padding-bottom: 8px; }
        .stat-row { display: flex; justify-content: space-between; margin-bottom: 10px; font-size: 14px; }
        .stat-label { color: #8c9ba5; }
        .stat-value { font-weight: 600; color: #ffffff; }
        .badge { display: inline-block; padding: 4px 10px; border-radius: 6px; font-size: 12px; font-weight: bold; text-transform: uppercase; margin-top: 10px; }
        .badge-fail { background: rgba(255, 75, 75, 0.2); color: #ff4b4b; border: 1px solid #ff4b4b; }
        .badge-success { background: rgba(46, 213, 115, 0.2); color: #2ed573; border: 1px solid #2ed573; }
        #controls { position: absolute; bottom: 20px; left: 50%; transform: translateX(-50%); z-index: 10; background: rgba(18, 22, 28, 0.85); backdrop-filter: blur(12px); border: 1px solid rgba(255,255,255,0.15); border-radius: 30px; padding: 10px 20px; display: flex; gap: 10px; }
        button { background: #2f3640; color: #fff; border: 1px solid rgba(255,255,255,0.2); padding: 8px 16px; border-radius: 20px; cursor: pointer; font-size: 13px; transition: all 0.2s; }
        button:hover { background: #4facfe; border-color: #4facfe; color: #000; font-weight: bold; }
        button.active { background: #4facfe; border-color: #4facfe; color: #000; font-weight: bold; }
        #info-box { position: absolute; top: 20px; right: 20px; z-index: 10; background: rgba(18, 22, 28, 0.85); backdrop-filter: blur(12px); border: 1px solid rgba(255,255,255,0.15); border-radius: 12px; padding: 20px; width: 320px; font-size: 13px; line-height: 1.5; color: #a1b0b8; }
        .highlight { color: #ffffff; font-weight: bold; }
    </style>
</head>
<body>
    <div id="canvas-container"></div>
    
    <div id="hud">
        <h1>Simulated Impact HUD</h1>
        <div class="stat-row"><span class="stat-label">Scenario:</span><span class="stat-value" id="hud-scenario">-</span></div>
        <div class="stat-row"><span class="stat-label">Impact Velocity:</span><span class="stat-value" id="hud-velocity">-</span></div>
        <div class="stat-row"><span class="stat-label">Mach Number:</span><span class="stat-value" id="hud-mach">-</span></div>
        <div class="stat-row"><span class="stat-label">Kinetic Energy:</span><span class="stat-value" id="hud-energy">-</span></div>
        <div class="stat-row"><span class="stat-label">Dynamic Pressure:</span><span class="stat-value" id="hud-pressure">-</span></div>
        <div class="stat-row"><span class="stat-label">Casing Yield Strength:</span><span class="stat-value" id="hud-yield">2.00 GPa</span></div>
        <div class="stat-row"><span class="stat-label">Hydro Depth Limit (P):</span><span class="stat-value" id="hud-depth">-</span></div>
        <div id="hud-badge-container"><span class="badge" id="hud-badge">-</span></div>
    </div>

    <div id="info-box">
        <h2 style="font-size:15px; margin:0 0 10px 0; color:#fff;">3D Physical Behavior</h2>
        <p id="info-text">Select a scenario below to visualize the impact regime.</p>
        <hr style="border:0; border-top:1px solid rgba(255,255,255,0.1); margin:12px 0;">
        <p style="font-size:11px; color:#718093;">Orbit: Left-Click + Drag<br>Pan: Right-Click + Drag<br>Zoom: Scroll Wheel</p>
    </div>

    <div id="controls">
)";

    for (size_t i = 0; i < results.size(); ++i) {
        out << "        <button onclick=\"selectScenario(" << i << ")\" id=\"btn-" << i << "\">" 
            << results[i].scenario_name << "</button>\n";
    }

    out << R"(    </div>

    <!-- Three.js and OrbitControls -->
    <script src="https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/three@0.128.0/examples/js/controls/OrbitControls.js"></script>
    <script>
        // Simulation Data from C++
        const scenarios = [
)";

    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        out << "            { name: \"" << r.scenario_name << "\", velocity: " << r.velocity 
            << ", mach: " << r.mach_number << ", energy: " << (r.kinetic_energy/1e9) 
            << ", pressure: " << (r.dynamic_pressure/1e9) << ", yield: " << (proj.yield_strength/1e9)
            << ", depth: " << r.hydro_penetration << ", fail: " << (r.casing_failure ? "true" : "false") 
            << ", regime: \"" << r.regime << "\", summary: \"" << r.outcome_summary << "\" }";
        if (i + 1 < results.size()) out << ",";
        out << "\n";
    }

    out << R"(        ];

        let scene, camera, renderer, controls;
        let targetBlock, projectile, shockwave, crater;

        function init() {
            const container = document.getElementById('canvas-container');
            scene = new THREE.Scene();
            scene.background = new THREE.Color(0x0a0c0e);
            scene.fog = new THREE.FogExp2(0x0a0c0e, 0.015);

            camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 0.1, 1000);
            camera.position.set(25, 20, 35);

            renderer = new THREE.WebGLRenderer({ antialias: true });
            renderer.setSize(window.innerWidth, window.innerHeight);
            renderer.shadowMap.enabled = true;
            container.appendChild(renderer.domElement);

            controls = new THREE.OrbitControls(camera, renderer.domElement);
            controls.enableDamping = true;
            controls.dampingFactor = 0.05;
            controls.target.set(0, -5, 0);

            // Lighting
            const ambientLight = new THREE.AmbientLight(0xffffff, 0.4);
            scene.add(ambientLight);

            const dirLight = new THREE.DirectionalLight(0xffffff, 0.8);
            dirLight.position.set(20, 40, 20);
            dirLight.castShadow = true;
            scene.add(dirLight);

            const pointLight = new THREE.PointLight(0x4facfe, 1, 50);
            pointLight.position.set(-10, 15, -10);
            scene.add(pointLight);

            // Grid / Ground
            const gridHelper = new THREE.GridHelper(60, 60, 0x334155, 0x1e293b);
            gridHelper.position.y = 0;
            scene.add(gridHelper);

            // Target Concrete Block (Translucent Cube)
            const blockGeom = new THREE.BoxGeometry(20, 25, 20);
            const blockMat = new THREE.MeshPhysicalMaterial({
                color: 0x475569,
                transparent: true,
                opacity: 0.35,
                roughness: 0.7,
                metalness: 0.1,
                wireframe: false
            });
            targetBlock = new THREE.Mesh(blockGeom, blockMat);
            targetBlock.position.set(0, -12.5, 0); // Top surface at y = 0
            scene.add(targetBlock);

            // Target surface wireframe box for visual structure
            const edges = new THREE.EdgesGeometry(blockGeom);
            const line = new THREE.LineSegments(edges, new THREE.LineBasicMaterial({ color: 0x64748b, linewidth: 2 }));
            targetBlock.add(line);

            // Projectile (Cylinder)
            const projGeom = new THREE.CylinderGeometry(0.6, 0.6, 6.2, 32);
            const projMat = new THREE.MeshStandardMaterial({ color: 0xe2e8f0, metalness: 0.9, roughness: 0.2 });
            projectile = new THREE.Mesh(projGeom, projMat);
            scene.add(projectile);

            // Crater / Erosion zone representation
            const craterGeom = new THREE.CylinderGeometry(1.2, 0.4, 11, 32);
            const craterMat = new THREE.MeshBasicMaterial({ color: 0xff4b4b, wireframe: true, transparent: true, opacity: 0.7 });
            crater = new THREE.Mesh(craterGeom, craterMat);
            scene.add(crater);

            // Shockwave Sphere (For hypervelocity failure)
            const shockGeom = new THREE.SphereGeometry(3, 32, 32);
            const shockMat = new THREE.MeshBasicMaterial({ color: 0xff0000, transparent: true, opacity: 0.4, wireframe: true });
            shockwave = new THREE.Mesh(shockGeom, shockMat);
            shockwave.position.set(0, 0, 0);
            scene.add(shockwave);

            window.addEventListener('resize', onWindowResize);
            selectScenario(0);
            animate();
        }

        function selectScenario(index) {
            const data = scenarios[index];
            document.querySelectorAll('#controls button').forEach((btn, i) => {
                btn.className = (i === index) ? 'active' : '';
            });

            document.getElementById('hud-scenario').innerText = data.name;
            document.getElementById('hud-velocity').innerText = data.velocity.toFixed(1) + ' m/s';
            document.getElementById('hud-mach').innerText = 'Mach ' + data.mach.toFixed(1);
            document.getElementById('hud-energy').innerText = data.energy.toFixed(2) + ' GJ';
            document.getElementById('hud-pressure').innerText = data.pressure.toFixed(2) + ' GPa';
            document.getElementById('hud-depth').innerText = data.depth.toFixed(2) + ' m (' + (data.depth * 3.28084).toFixed(1) + ' ft)';
            
            const badge = document.getElementById('hud-badge');
            if (data.fail) {
                badge.innerText = 'HYDRODYNAMIC FAILURE (P > YIELD)';
                badge.className = 'badge badge-fail';
                
                // 3D Visual update for Hypervelocity Failure
                projectile.position.set(0, -data.depth / 2, 0); // Erode into the crater
                projectile.scale.set(1.5, 0.5, 1.5); // Crushed/shattered shape
                projectile.material.color.setHex(0xff6b6b);
                
                crater.visible = true;
                crater.scale.set(1, data.depth / 11.0, 1);
                crater.position.set(0, -data.depth / 2, 0);
                crater.material.color.setHex(0xff4b4b);

                shockwave.visible = true;
                shockwave.scale.set(1.8, 0.8, 1.8);

                document.getElementById('info-text').innerHTML = 
                    `<span class="highlight">Dynamic Pressure (${data.pressure.toFixed(2)} GPa) exceeds Yield Strength (${data.yield.toFixed(2)} GPa).</span><br><br>` +
                    `The impact shockwave instantly crushes the casing and causes premature detonation near the surface. Penetration is capped by fluid erosion at ~${data.depth.toFixed(2)}m.`;
            } else {
                badge.innerText = 'RIGID DRILL PENETRATION (SUCCESS)';
                badge.className = 'badge badge-success';
                
                // 3D Visual update for Subsonic Rigid Penetration
                projectile.position.set(0, -6.0, 0); // Drilling cleanly deep underground
                projectile.scale.set(1, 1, 1); // Intact shape
                projectile.material.color.setHex(0x38bdf8);
                
                crater.visible = true;
                crater.scale.set(0.6, 2.0, 0.6); // Deep clean drill hole
                crater.position.set(0, -10.0, 0);
                crater.material.color.setHex(0x38bdf8);

                shockwave.visible = false;

                document.getElementById('info-text').innerHTML = 
                    `<span class="highlight">Dynamic Pressure (${data.pressure.toFixed(2)} GPa) stays below Yield Strength (${data.yield.toFixed(2)} GPa).</span><br><br>` +
                    `The steel casing remains rigid and intact like a drill bit. The smart-fuze survives impact and carries the payload deep underground (40-60+ meters) before detonation.`;
            }
        }

        function onWindowResize() {
            camera.aspect = window.innerWidth / window.innerHeight;
            camera.updateProjectionMatrix();
            renderer.setSize(window.innerWidth, window.innerHeight);
        }

        function animate() {
            requestAnimationFrame(animate);
            if (shockwave.visible) {
                shockwave.rotation.y += 0.01;
                shockwave.rotation.x += 0.005;
            }
            controls.update();
            renderer.render(scene, camera);
        }

        window.onload = init;
    </script>
</body>
</html>
)";
    out.close();
    std::cout << "\n[+] Successfully generated 3D WebGL Interactive Visualizer: " << filename << "\n";
    std::cout << "    -> Open " << filename << " in your web browser to view the 3D simulation scene!\n";
}
