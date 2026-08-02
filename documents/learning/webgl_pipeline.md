# WebGL 3D Visualization Pipeline

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

## Architecture
`MOP Simulator` generates an interactive Three.js 3D visualization without needing a web server or backend API.

## Data Injection Mechanism
1. `ImpactSimulator::generateHtml3DVisualizer` loads `assets/visualizer_template.html`.
2. The simulation trajectory slice data is serialized into JSON-compatible arrays (`crossSectionsData`, `depthsData`, `velocitiesData`).
3. The C++ engine replaces template placeholder comment tokens inside the HTML file string and writes the output directly to `output_3d_visualizer.html`.
