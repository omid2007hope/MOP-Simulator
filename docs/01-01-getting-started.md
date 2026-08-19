# 1.1 Getting Started

// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

This guide walks through configuring the prerequisites, compiling the C++23 native simulation engine, configuring the Node.js automation layer, and executing autonomous simulation sweeps.

---

## 💻 System Prerequisites

| Dependency | Minimum Version | Recommended | Notes |
| :--- | :--- | :--- | :--- |
| **C++ Compiler** | GCC 13.0+ / Clang 17+ | MinGW-w64 GCC 14.2 (C++23) | Supports `<numbers>`, `<expected>`, `std::format` |
| **Build Tool** | GNU Make 4.0+ | `mingw32-make` / `make` | Makefile included in `backend/` |
| **Node.js** | v20.0.0+ | v24.x LTS | Required for ES Modules & Async Streams |
| **Database** | MongoDB 6.0+ | MongoDB 7.0+ (Local / Atlas) | Telemetry persistence & aggregation |
| **AI Access** | Google Gemini API | Gemini 2.5 Flash API Key | Optional (deterministic fallback built-in) |

---

## 🛠️ Step 1: Compile the C++ Simulation Kernel

Open a PowerShell or Bash terminal in the `backend/` directory:

```powershell
cd backend

# Clean existing build artifacts and compile with C++23 optimizations
mingw32-make clean
mingw32-make
```

Upon successful compilation, the native executable is generated at:
`backend/bin/mop_sim.exe` (Windows) or `backend/bin/mop_sim` (Linux/macOS).

### Verify Binary Execution:

```powershell
# Run the built-in C++ test suite
./bin/mop_sim.exe --test
```

---

## 📦 Step 2: Configure Node.js & AI Environment

1. Navigate to the automation backend directory:

```powershell
cd src/Automation
```

2. Install Node.js dependencies:

```powershell
npm install
```

3. Create the environment configuration file (`.env`):

```powershell
# Windows PowerShell
@"
PORT=3000
MONGO_URI=mongodb://127.0.0.1:27017/mop-simulator
GEMINI_API_KEY=your_gemini_api_key_here
"@ | Out-File -Encoding utf8 .env
```

> [!TIP]
> **Deterministic Fallback**: If you do not have a `GEMINI_API_KEY`, you can leave it blank. The system will automatically engage high-fidelity deterministic mock generators for scenarios and articles without throwing errors.

---

## 🚀 Step 3: Launch the Automation Backend

Ensure your MongoDB instance is running, then start the Express server:

```powershell
npm start
```

You should see output similar to:

```text
MongoDB connected: mongodb://127.0.0.1:27017/mop-simulator
Server running on http://localhost:3000
```

---

## 🧪 Step 4: Execute Your First Autonomous Campaign

### 1. Trigger Simulation Cycles (`POST /research`)

```bash
curl -X POST http://localhost:3000/research \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Evaluation of 70MPa Reinforced Concrete Penetration",
    "description": "GBU-57 MOP casing survivability and penetration depth analysis.",
    "count": 3
  }'
```

**Expected Response (`200 OK`):**

```json
{
  "data": {
    "message": "Autonomous cycles finished",
    "session_id": "8f3b21a9",
    "cycles": [
      { "cycle": 1, "frames_saved": 1240, "status": "success" },
      { "cycle": 2, "frames_saved": 1185, "status": "success" },
      { "cycle": 3, "frames_saved": 1210, "status": "success" }
    ]
  }
}
```

### 2. Synthesize Academic Research Article (`POST /article`)

Using the `session_id` returned from Step 1:

```bash
curl -X POST http://localhost:3000/article \
  -H "Content-Type: application/json" \
  -d '{
    "session_id": "8f3b21a9",
    "limit": 500
  }'
```

The system will aggregate the telemetry across MongoDB, run statistical evaluations, and generate a complete academic paper with abstract, methodology, results, and citations.

---

## 🧭 Navigation

* [Back to 1. Project Overview](01-overview.md)
* [Proceed to 1.2 Project Roadmap & Contribution Guide](01-02-roadmap-and-contribution.md)
* [Explore 2. System Architecture](02-system-architecture.md)
