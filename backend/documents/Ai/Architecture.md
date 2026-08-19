# AI & System Architectural Directives

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

## System Hierarchy

```
  ┌─────────────────────────────────────────────────────────────┐
  │ 1. AI Orchestrator Layer (src/AI)                          │
  │    - aiClient.js (Gemini Flash 2.5 REST Client)             │
  │    - Prompt.js (researchConductor, articleWriter)          │
  └──────────────────────────────┬──────────────────────────────┘
                                 │ JSON Directives / Articles
  ┌──────────────────────────────▼──────────────────────────────┐
  │ 2. Automation & Database Layer (src/Automation)             │
  │    - server.js (Express REST API)                           │
  │    - service/research.js (Multi-cycle Loop Orchestrator)    │
  │    - service/simulationRunner.js (IPC Bridge & Chunk Stream)│
  │    - service/articleWriter.js (Statistical Analysis)        │
  │    - model/result.js & model/article.js (MongoDB Mongoose)  │
  └──────────────────────────────┬──────────────────────────────┘
                                 │ CLI Invocation (--json-input)
  ┌──────────────────────────────▼──────────────────────────────┐
  │ 3. Native C++23 Physics Engine (src/simulation)             │
  │    - main.cpp (CLI Entry, JSON/Interactive Dispatcher)       │
  │    - simulation.cpp (RK4 Integrator, WAPM, Hugoniot, DIF)   │
  │    - telemetry_exporter.cpp (JSON Stdout Stream & HTML 3D)  │
  │    - config_loader.cpp (JSON Database Parser)               │
  └─────────────────────────────────────────────────────────────┘
```

## Hard Architectural Rules
- **Pure Physics Boundary**: `ImpactSimulator` focuses exclusively on deterministic numerical integration and telemetry frame generation. It does not perform database operations or direct network calls.
- **Session Scoping**: All telemetry frames stored in MongoDB must be scoped by `research_title` and `session_id` to prevent cross-experiment contamination.
- **Memory Safety & Streaming**: Never buffer complete multi-thousand frame telemetry sets in memory. Stream line-by-line and batch insert in chunks of 1,000.
- **No Circular Dependencies**: `src/simulation` must never depend on Node.js layers; `src/Automation` communicates exclusively via OS child process boundaries (`spawn`) and temporary JSON payloads.
