# AI Architectural Directives

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

## System Hierarchy
1. **Config Layer** (`include/config_loader.hpp`, `src/config_loader.cpp`):
   - Reads `data/targets.json` and `data/projectiles.json`.
   - Parses target/projectile database records using `nlohmann::json`.
2. **Physics Engine Layer** (`include/simulation.hpp`, `src/simulation.cpp`):
   - Pure, stateless simulation logic (`ImpactSimulator::simulate`).
   - Receives an `ImpactScenario` struct and returns a comprehensive `SimulationResult`.
   - Executes structural integrity checks, time-integrated deceleration steps, thermal/cook-off thresholds, and HTML generation.
3. **Application Entry / CLI Layer** (`src/main.cpp`):
   - Handles legal consent (EULA/ToS), user choices, preset/interactive prompts, and console UI reporting.

## Hard Rules
- **No Circular Dependencies**: `simulation` must never depend on `main.cpp`.
- **Pure Physics Functions**: `ImpactSimulator` should focus exclusively on numerical integration, damage evaluation, and visualization array building.
