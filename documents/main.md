# Application Entry & Data Flow (`main.cpp` to `simulation.cpp`)

The application entry point (`main.cpp`) supports two operational modes:
1. **Headless JSON Mode (`--json-input`)**: For autonomous AI sweeps and Node.js backend integration.
2. **Interactive Console Mode**: For manual user interaction with preset selection and custom terminal inputs.

---

## 1. Headless JSON Execution Flow (`--json-input`)

When launched with `--json-input <path_to_config.json>`, `main.cpp`:
1. Reads and parses the JSON configuration using `nlohmann::json`.
2. Extracts projectile geometry, casing metallurgy, explosive properties, target strata, and kinematic scenario parameters.
3. Automatically builds the `Projectile`, `Target`, and `ImpactScenario` structures.
4. Executes the simulation without prompting for EULA/ToS or waiting for console keystrokes.
5. Emits line-delimited JSON telemetry to `stdout` for ingestion by `simulationRunner.js`.

```cpp
if (jsonMode) {
    std::ifstream ifs(jsonFile);
    json config;
    ifs >> config;

    int simChoice = config.value("/Simulation/choice"_json_pointer, 3);
    // Parse projectile, target layers, and scenario...
}
```

---

## 2. Interactive Console Data Flow

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

---

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

### Step-by-Step Flow:

1. **`main.cpp`**: Instantiates `ImpactSimulator simulator(munition, object, cons);` passing parameters.
2. **`simulation.hpp`**: Declares constructor contract `ImpactSimulator(const Projectile& p, const Target& t, const PhysicsConstants& c);`.
3. **`simulation.cpp`**: Executes initializer list copying `p -> proj`, `t -> target`, `c -> cons` into private members.
4. **`ImpactSimulator::simulate(sc)`**: Runs 2-phase RK4 integration (atmospheric drop + ground penetration) and returns `SimulationResult`.
5. **`TelemetryExporter::printReport`**: Prints ASCII visualizer and outputs full line-delimited JSON strings matching `ResultModel` schema.
