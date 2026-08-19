# 2.2.1 REST API Endpoints & Controllers

// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

---

## 📡 REST API Reference

The Node.js automation layer exposes a clean, RESTful API on default port `3000`.

---

### 1. `POST /research` — Run Autonomous Simulation Campaign

Initiates a multi-cycle closed-loop simulation campaign. The AI Research Conductor generates physical variations of the topic, executes the C++ engine for each cycle, and streams telemetry into MongoDB.

* **Endpoint:** `POST /research`
* **Content-Type:** `application/json`

#### Request Body Schema:

| Field | Type | Required | Default | Description |
| :--- | :--- | :--- | :--- | :--- |
| `title` | `string` | **Yes** | — | High-level research topic or scenario description. |
| `description` | `string` | No | `""` | Additional physical constraints or contextual details. |
| `count` | `number` | No | `1` | Number of autonomous simulation iterations (1–20). |

#### Request Example:

```json
{
  "title": "Optimizing Casing Thickness for 70MPa Concrete",
  "description": "Parametric evaluation of GBU-57 MOP casing wall thickness variations against ultra-high performance reinforced concrete bunkers.",
  "count": 3
}
```

#### Response Example (`200 OK`):

```json
{
  "data": {
    "message": "Autonomous cycles finished",
    "session_id": "a4f8b91c",
    "cycles": [
      { "cycle": 1, "frames_saved": 1240, "status": "success" },
      { "cycle": 2, "frames_saved": 1185, "status": "success" },
      { "cycle": 3, "frames_saved": 1210, "status": "success" }
    ]
  }
}
```

#### Error Responses:
* `400 Bad Request`: Missing `title` parameter.
* `500 Internal Server Error`: C++ binary execution failure or database write timeout.

---

### 2. `POST /article` — Synthesize Academic Research Article

Queries all telemetry and simulation outcomes stored under a specific `session_id`, computes comprehensive statistical metrics, and prompts the AI Article Writer to produce a publication-grade paper.

* **Endpoint:** `POST /article`
* **Content-Type:** `application/json`

#### Request Body Schema:

| Field | Type | Required | Default | Description |
| :--- | :--- | :--- | :--- | :--- |
| `session_id` | `string` | **Yes** | — | Cryptographic session identifier returned by `/research`. |
| `limit` | `number` | No | `500` | Maximum number of simulation records to sample for statistics. |

#### Request Example:

```json
{
  "session_id": "a4f8b91c",
  "limit": 500
}
```

#### Response Example (`201 Created`):

```json
{
  "data": {
    "article_id": "6a85591eea0eecc59e063895",
    "title": "Optimizing Casing Thickness for 70MPa Concrete",
    "session_id": "a4f8b91c",
    "word_count": 1406,
    "scenarios_analyzed": 6,
    "stats": {
      "totalScenarios": 6,
      "avgPenetrationDepth": "8.22",
      "maxPenetrationDepth": "10.27",
      "minPenetrationDepth": "6.17",
      "stdDevPenetration": "2.05",
      "avgVelocity": "537.4",
      "avgMach": "1.58",
      "avgEnergyGJ": "1.960",
      "avgShockPressureGPa": "3.620",
      "casingFailureRate": "0.0",
      "erosionRate": "0.0",
      "dominantRegime": "Rigid Penetration (Crater+Tunnel)",
      "regimeDistribution": {
        "Rigid Penetration (Crater+Tunnel)": 6
      }
    },
    "key_findings": [
      "Mean penetration depth: 8.22 m (σ = 2.05 m)",
      "Dominant regime: Rigid Penetration (Crater+Tunnel) in 100.0% of scenarios",
      "Casing integrity maintained in 100% of scenarios",
      "Maximum sequential breach depth: 10.27 m",
      "Hydrodynamic erosion onset in 0.0% of scenarios",
      "Average impact velocity: 537.4 m/s at Mach 1.58"
    ],
    "content": "# Optimizing Casing Thickness for 70MPa Concrete\n\n**MOP Simulator Autonomous Research Division**\n**Date:** August 19, 2026\n**Simulation Engine:** MOP Impact Physics & Penetration Simulator V3.5.0\n**Total Scenarios:** 6\n\n---\n\n## Abstract\n\nThis study presents a high-fidelity computational analysis..."
  }
}
```

#### Error Responses:
* `400 Bad Request`: Missing `session_id`.
* `404 Not Found`: No simulation records found for the provided `session_id`.
* `500 Internal Server Error`: AI generation or database persistence failure.

---

### 3. `GET /health` — Service Health & Uptime

Returns the operational status of the Express server, database connectivity, and environment state.

* **Endpoint:** `GET /health`

#### Response Example (`200 OK`):

```json
{
  "status": "online",
  "database": "connected",
  "version": "3.5.0",
  "timestamp": "2026-08-19T21:50:00.000Z"
}
```

---

## 🧭 Navigation

* [Back to 2.2 Node.js Automation Layer](02-02-nodejs-automation-layer.md)
* [Proceed to 2.2.2 Services: SimulationRunner & ArticleWriter](02-02-02-services-simulation-runner-and-article-writer.md)
* [Explore 2.2.3 MongoDB Data Models](02-02-03-mongodb-data-models.md)
