# MOP-Simulator Architecture

## AI Layer (Node.js)

### Data Modeling & Generation

The AI layer is responsible for translating the intended research context (title and description)
into concrete simulation data. Rather than generating entirely random values, the AI intelligently
experiments and iterates on parameters to explore different outcomes, assess results, and draw
conclusions—effectively acting as an automated researcher.

#### Raw Data Generation

The AI generates the foundational raw values for the simulation runs based on defined cycles and
scenarios.

- **Example:** 3 cycles with 5 scenarios each results in 15 total simulation scenarios (3 datasets ×
  5 scenarios).

#### Core Models

The AI constructs and populates the following structural models for the simulation:

- `Scenarios` (Array)
- `PhysicsConstants`
- `AtmosphereState`
- `Aircraft`
- `Projectile`
- `Target`

## Data Flow & Execution Pipeline

1. **Raw Data Storage:** The AI-generated raw datasets are saved into the database.
2. **Simulation Execution:** The data is fed into the C++ Core executable for processing.
3. **Result Generation:** The C++ Core executes the simulation and outputs a result dataset.
4. **Data Linkage:** The final results are saved back into the database with a direct reference
   linking them to their corresponding raw dataset.

## Post-Simulation Analysis (AI Layer)

### Research Analysis

* **Schema Model:** `researchSessionSchema`

The AI analyzes the generated result datasets to extract meaningful insights, producing comprehensive and detailed research texts.

### Article Generation

* **Schema Model:** `articleSchema`

Based on the synthesized research texts, the AI constructs and outputs a high-quality, formatted scientific article.

## Simulation Core (C++ Executable)

### Key Responsibilities

- **Configuration Loading:** Utilizes a `Config Loader` to fetch and parse the AI-generated raw
  datasets from the database.
- **Execution:** Runs the high-performance physical simulation based on the loaded models and
  parameters.
- **Data Export:** Transmits the generated simulation results back to the Node.js layer for
  persistent storage in the database.
