#include <iostream>
#include <cassert>
#include <fstream>
#include "nlohmann/json.hpp"
#include "penetration/versionOne/simulation.hpp"
#include "penetration/versionOne/telemetry_exporter.hpp"

using json = nlohmann::json;

void runTest() {
    std::cout << "[TEST] Telemetry Render Validation...\n";
    
    Projectile p{"Test Bomb", 5.0, 0.5, 1.0, 10000.0, 2000.0, 5e6, 7800.0, 1.2e9, 0.015, 210e9, 0.04, 4500.0, 1.5, 2e15, 450.0, 1700.0, 270000.0};
    TargetLayer layer{"Concrete", 10.0, 0.03, 400e6, 2400.0, 50e6, 3000.0, 1.8, 850.0, 1600.0, 350000.0};
    Target t{"Bunker", {layer}};
    Aircraft a{"Test Bomber", 150000.0, 450.0, 4.0};
    AtmosphereState atmos{300.0, 100000.0, 1.2, 340.0};
    PhysicsConstants cons;

    ImpactSimulator simulator(p, t, cons);
    ImpactScenario sc{"Test Scenario", 30000.0, 0.0, 90.0, 0.0, 0.0};
    
    std::vector<SimulationResult> results;
    results.push_back(simulator.simulate(sc, a, atmos));

    // Generate HTML visualizer
    TelemetryExporter::generateHtml3DVisualizer(results, p, t, ".");
    
    // Validate output file exists
    std::ifstream htmlFile("./MindMap.html");
    assert(htmlFile.is_open()); // File was successfully created
    
    std::string htmlContent((std::istreambuf_iterator<char>(htmlFile)), std::istreambuf_iterator<char>());
    htmlFile.close();
    
    // Ensure critical JSON payload markers are present
    assert(htmlContent.find("const projectileData = {") != std::string::npos);
    assert(htmlContent.find("const targetData = {") != std::string::npos);
    assert(htmlContent.find("const simulationResults = [") != std::string::npos);
    
    // Check if lengths and widths match our setup
    assert(htmlContent.find("\"length\": 5.0") != std::string::npos);
    
    std::cout << "[TEST] Telemetry Render Validation PASSED!\n";
}

int main() {
    runTest();
    return 0;
}
