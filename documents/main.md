# Data Flow Overview (`main.cpp` to `simulation.cpp`)

The `scenarios` variable is a dynamic list (`std::vector<ImpactScenario>`) of struct instances from `simulation.hpp` that holds all simulation test conditions.

## 1. Setting Up Target & Projectile Data

```cpp
// Target & Projectile objects (editable variables)
Target object;
object.name = "...";

Projectile munition;
munition.total_mass = ...;

// Adding custom target layers
object.layers.clear();
TargetLayer customLayer;
customLayer.material_name = "Custom Layer";
customLayer.thickness = ...;
object.layers.push_back(customLayer);
```

## 2. Populating Test Scenarios

```cpp
std::stringstream prompt_ss;
std::stringstream name_ss;

scenarios.push_back({
    name_ss.str(),
    dropAltitude_ft,
    initial_velocity,
    fpa,
    obliquity,
    aoa
});
```

## 3. Passing Data into `ImpactSimulator` (`simulation.cpp`)

```cpp
// Evaluate single vs sequential simulation modes
if (choice == 1 || choice == 2) {
    // Independent tests get fresh target objects
    for (const auto& sc : scenarios) {
        ImpactSimulator simulator(munition, object, cons);
        results.push_back(simulator.simulate(sc));
    }

    TelemetryExporter::printReport(results, munition, object);
    TelemetryExporter::generateHtml3DVisualizer(results, munition, object, basePath);
}
```
