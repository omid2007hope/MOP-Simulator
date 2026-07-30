# // Copyright (c) 2026 Omid Teimory. All Rights Reserved

import sys

file_path = r"h:\Code\MyOwn\Main\Best\MOP Simulator\src\simulation.cpp"
with open(file_path, "r", encoding="utf-8") as f:
    content = f.read()

start_idx = content.find("void ImpactSimulator::generateHtml3DVisualizer(const std::vector<SimulationResult>& results)")
if start_idx == -1:
    print("Method not found")
    sys.exit(1)

new_method = """void ImpactSimulator::generateHtml3DVisualizer(const std::vector<SimulationResult>& results)
{
    std::string filename = "3d_visualizer.html";
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error: Could not open " << filename << " for writing.\\n";
        return;
    }

    std::ifstream tpl("assets/visualizer_template.html");
    if (!tpl.is_open()) {
        std::cerr << "Error: Could not open template file assets/visualizer_template.html\\n";
        return;
    }

    std::stringstream buffer;
    buffer << tpl.rdbuf();
    std::string html = buffer.str();

    // Generate scenario buttons
    std::stringstream buttons;
    for (size_t i = 0; i < results.size(); ++i) {
        buttons << "        <button onclick=\\"selectScenario(" << i << ")\\" id=\\"btn-" << i
            << "\\" class=\\"px-4 py-1.5 rounded-full text-xs font-semibold whitespace-nowrap "
               "transition-all duration-200 bg-slate-800/80 hover:bg-cyan-500 "
               "hover:text-slate-950 border border-slate-700/60\\">"
            << results[i].scenario_name << "</button>\\n";
    }

    // Generate scenario data
    std::stringstream data;
    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        data << "            { name: \\"" << r.scenario_name << "\\", velocity: " << r.velocity
            << ", mach: " << r.mach_number << ", energy: " << (r.kinetic_energy / 1e9)
            << ", pressure: " << (r.dynamic_pressure / 1e9)
            << ", yield: " << (proj.yield_strength / 1e9)
            << ", depth: " << r.actual_penetration_depth
            << ", rigid_depth: " << r.rigid_penetration
            << ", hydro_depth: " << r.hydro_penetration
            << ", fail: " << (r.casing_failure ? "true" : "false")
            << ", shock_prob: " << r.shock_damage_prob_percent
            << ", exp_survives: " << (r.explosive_charge_survives ? "true" : "false")
            << ", is_kinetic: " << (r.is_kinetic_rod ? "true" : "false") << ", regime: \\""
            << r.regime << "\\", summary: \\"" << r.outcome_summary << "\\" }";
        if (i + 1 < results.size())
            data << ",";
        data << "\\n";
    }

    auto replaceAll = [](std::string& str, const std::string& from, const std::string& to) {
        if(from.empty()) return;
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); 
        }
    };

    replaceAll(html, "{{SCENARIO_BUTTONS}}", buttons.str());
    replaceAll(html, "{{SCENARIOS_DATA}}", data.str());
    replaceAll(html, "{{PROJ_NAME}}", proj.name);
    replaceAll(html, "{{PROJ_LENGTH}}", std::to_string(proj.length));
    replaceAll(html, "{{PROJ_DIAMETER}}", std::to_string(proj.diameter));
    replaceAll(html, "{{PROJ_MASS}}", std::to_string(proj.total_mass));
    replaceAll(html, "{{PROJ_EXPLOSIVE_MASS}}", std::to_string(proj.explosive_mass));
    replaceAll(html, "{{PROJ_CASING_DENSITY}}", std::to_string(proj.casing_density));
    replaceAll(html, "{{PROJ_YIELD_STRENGTH}}", std::to_string(proj.yield_strength));
    replaceAll(html, "{{TARGET_NAME}}", target.name);
    replaceAll(html, "{{TARGET_DENSITY}}", std::to_string(target.density));
    replaceAll(html, "{{TARGET_BEARING_STRENGTH}}", std::to_string(100.0e6));
    replaceAll(html, "{{DRAG_COEFFICIENT}}", std::to_string(1.2));
    replaceAll(html, "{{SPEED_OF_SOUND}}", std::to_string(SPEED_OF_SOUND));

    out << html;
    out.close();

    std::cout << "\\n[+] Successfully generated 3D WebGL Interactive Visualizer: " << filename
              << "\\n";
    std::cout << "    -> Open " << filename
              << " in your web browser to view the 3D simulation scene!\\n";
}
"""

with open(file_path, "w", encoding="utf-8") as f:
    f.write(content[:start_idx] + new_method)

print("Replacement successful")
