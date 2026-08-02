# Command Reference: Deployment & Packaging

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

## Building Production Release Executable
To compile an optimized release binary:

```powershell
mingw32-make clean
mingw32-make
```

## Distribution Package Layout
When releasing `MOP Simulator V2.8`, package the directory with the following structure:

```text
MOP_Simulator_v2.8/
├── bin/
│   └── sim.exe
├── data/
│   ├── targets.json
│   └── projectiles.json
├── assets/
│   └── visualizer_template.html
└── README.md
```
