# 2. System Architecture

// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

---

## 🏛️ Multi-Tier Architecture Overview

The **MOP Simulator** platform is engineered as a decoupled, multi-tier computational architecture bridging external AI/LLM models, a high-throughput Node.js orchestration server, a low-latency native C++23 continuum mechanics engine, and high-fidelity rendering engines.

```mermaid
flowchart TD
    subgraph AILayer["🧠 1. AI Orchestrator Layer (src/AI)"]
        A1[Research Conductor<br/>Scenario Generator] -->|Hypothesis Config| A2[aiClient.js<br/>Gemini 2.5 Flash]
        A3[Article Writer<br/>Paper Synthesizer] <--|Aggregated Statistics| A2
    end

    subgraph AutomationLayer["🌐 2. Automation & Database Layer (src/Automation)"]
        N1[Express.js REST API<br/>router / controller]
        N2[SimulationRunner Service<br/>Process Lifecycle & Watchdog]
        N3[ArticleWriter Service<br/>Statistical Engine]
        N4[(MongoDB Database<br/>Result & Article Collections)]
        
        N1 <--> N2
        N1 <--> N3
        N2 -->|Asynchronous Stream & Batch Ingestion| N4
        N3 <-->|Aggregated Queries| N4
    end

    subgraph CppLayer["⚙️ 3. Native C++23 Physics Engine (src/simulation)"]
        C1[CLI Dispatcher / main.cpp<br/>--json-input / Console Mode]
        C2[ImpactSimulator Core<br/>RK4, Forrestal, WAPM, Hugoniot]
        C3[Telemetry Exporter<br/>Line-Delimited JSON stdout Stream]
        
        C1 --> C2 --> C3
    end

    subgraph VisualizerLayer["🎨 4. Rendering & Visualization Layer"]
        V1[Interactive 3D WebGL / Three.js<br/>3d_visualizer.html]
        V2[Unreal Engine 5 Bridge<br/>MOPSimulationBridge / UActorComponent]
        
        C3 -.->|HTML Export| V1
        C2 -.->|Direct C++ Bridge| V2
    end

    A2 <-->|REST Requests| N1
    N2 -->|Spawn child process<br/>--json-input temp_config.json| C1
    C3 -->|High-Frequency Telemetry stdout| N2
```

---

## 🔁 Complete Data Lifecycle & IPC Pipeline

```
  [User / REST Client]
          │
          ▼  (1) POST /research { title, count }
  [AI Orchestrator (`src/AI`)]
          │
          ▼  (2) Generates dynamic physical scenario hypotheses
  [Node.js SimulationRunner (`src/Automation/service/simulationRunner.js`)]
          │
          ▼  (3) Serializes temp JSON config & spawns `bin/mop_sim.exe --json-input <tmp_file>`
  [C++ Physics Kernel (`src/simulation/penetration`)]
          │
          ▼  (4) Executes sub-millisecond RK4 integration & prints line-delimited JSON
  [Telemetry Stream (`stdout`)]
          │
          ▼  (5) Readline generator parses frames & buffers into 1,000-doc batches
  [MongoDB Database (`model/result.js`)]
          │
          ▼  (6) POST /article { session_id } triggers statistical aggregation
  [AI Article Synthesizer (`src/AI/Prompt.js`)]
          │
          ▼  (7) Generates 4,000+ word publication-grade academic article
  [MongoDB Article Collection (`model/article.js`)]
```

---

## 🔒 Architectural Boundaries & Invariants

1. **Pure Physics Boundary**:
   The native physics kernel (`ImpactSimulator`) is strictly deterministic and stateless per run. It has zero external dependencies on Node.js, databases, or networking libraries. It receives immutable data structures and outputs pure telemetry.

2. **Multi-Tenant Session Scoping**:
   All database records are strictly tagged with a cryptographic `session_id` and `research_title`. This completely eliminates data cross-contamination during concurrent multi-topic research campaigns.

3. **Memory Safety & Backpressure Handling**:
   The Node.js streaming parser never buffers unconstrained telemetry arrays in RAM. By utilizing asynchronous `readline` generators and chunking documents in batches of 1,000, Node.js memory footprint remains bounded under $50\text{ MB}$ even for runs producing $> 100,000$ telemetry frames.

4. **Watchdog Protection**:
   Every spawned simulation process is monitored by an active 30-second watchdog timer. If an extreme edge case causes numerical divergence or infinite loops, the child process is terminated immediately (`SIGKILL`), preventing server CPU lockups.

---

## 🧭 Subsystem Deep Dives

* [2.1 C++ Physics Engine (`src/simulation`)](02-01-cpp-physics-engine.md)
  * [2.1.1 Physics Models & Numerical Methods](02-01-01-physics-models-and-numerical-methods.md)
  * [2.1.2 Data Structures & Configuration Schema](02-01-02-data-structures-and-config-schema.md)
* [2.2 Node.js Automation Layer (`src/Automation`)](02-02-nodejs-automation-layer.md)
  * [2.2.1 REST API Endpoints & Controllers](02-02-01-rest-api-endpoints-and-controllers.md)
  * [2.2.2 Services: SimulationRunner & ArticleWriter](02-02-02-services-simulation-runner-and-article-writer.md)
  * [2.2.3 MongoDB Data Models](02-02-03-mongodb-data-models.md)
* [2.3 AI Orchestrator Layer (`src/AI`)](02-03-ai-orchestrator-layer.md)
* [3. 3D WebGL Visualizer](03-3d-webgl-visualizer.md)
* [4. Unreal Engine Integration](04-unreal-engine-integration.md)
