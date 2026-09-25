#include <iostream>
#include <cassert>
#include <fstream>
#include "nlohmann/json.hpp"
#include "penetration/versionOne/simulation.hpp"
#include "penetration/versionOne/telemetry_exporter.hpp"

using json = nlohmann::json;

void runTest() {
    std::cout << "[TEST] Telemetry Render Validation...\n";
    
    Projectile p{
        .name = "Test Bomb",
        .length = 5.0,
        .diameter = 0.5,
        .curvature_noseReduce = 1.0,
        .total_mass = 10000.0,
        .explosive_mass = 2000.0,
        .casing_density = 7800.0,
        .yield_strength = 1.2e9,
        .area_moment_inertia = 0.015,
        .elastic_modulus = 210e9,
        .casing_wall_thickness = 0.04,
        .hugoniot_c0 = 4500.0,
        .hugoniot_s = 1.5,
        .explosive_critical_energy = 2e15,
        .explosive_energy_j_per_kg = 5e6,
        .specific_heat = 450.0,
        .melting_point = 1700.0,
        .heat_of_fusion = 270000.0
    };
    TargetLayer layer{
        .material_name = "Concrete",
        .thickness = 10.0,
        .density = 2400.0,
        .compressive_strength = 50e6,
        .rebar_volume_fraction = 0.03,
        .rebar_yield_strength = 400e6,
        .pulverized_depth = 0.0,
        .hugoniot_c0 = 3000.0,
        .hugoniot_s = 1.8,
        .specific_heat = 850.0,
        .melting_point = 1600.0,
        .heat_of_fusion = 350000.0
    };
    Target t{"Bunker", {layer}};
    Aircraft a{"Test Bomber", 150000.0, 450.0, 4.0};
    AtmosphereState atmos{300.0, 100000.0, 1.2, 340.0};
    PhysicsConstants cons;

    ImpactSimulator simulator(p, t, cons);
    ImpactScenario sc{
        .name = "Test Scenario",
        .altitude_ft = 30000.0,
        .bombers_horizontal_velocity = 0.0,
        .velocity = 0.0,
        .flight_path_angle = 90.0,
        .obliquity_angle = 0.0,
        .angle_of_attack = 0.0
    };
    
    std::vector<SimulationResult> results;
    results.push_back(simulator.simulate(sc, a, atmos));

    // Generate HTML visualizer
    TelemetryExporter::generateHtml3DVisualizer(results, p, t, ".");
    
    // Validate output file exists
    std::ifstream htmlFile("./3d_visualizer.html");
    assert(htmlFile.is_open()); // File was successfully created
    
    std::string htmlContent((std::istreambuf_iterator<char>(htmlFile)), std::istreambuf_iterator<char>());
    htmlFile.close();
    
    // Ensure critical JSON payload markers are present in the serialized output
    assert(htmlContent.find("\"name\":\"Test Scenario\"") != std::string::npos);
    assert(htmlContent.find("\"proj_name\":\"Test Bomb\"") != std::string::npos);
    assert(htmlContent.find("\"target_layers\":") != std::string::npos);
    
    // Check if lengths and widths match our setup
    assert(htmlContent.find("\"proj_length\":5") != std::string::npos);
    
    std::cout << "[TEST] Telemetry Render Validation PASSED!\n";
}

int main() {
    runTest();
    return 0;
}
