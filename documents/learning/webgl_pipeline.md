# WebGL 3D Visualization Pipeline: C++ to Browser Bridge

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

---

## 1. Zero-Dependency Web Architecture

Most desktop software that renders 3D graphics either embeds a heavy web browser instance (like Electron/CEF) or runs a local web server (like Node.js/Express) to serve static files.

The **MOP Simulator** uses a much lighter and faster architectural pattern:
1. The C++ core physics engine performs high-precision Runge-Kutta numerical integration on the CPU.
2. The simulation outputs full state trajectories (`drop_frames` and `penetration_frames`).
3. C++ acts as a **template engine**, injecting structured JSON telemetry directly into an offline HTML WebGL template file (`assets/visualizer_template.html`).
4. The output `3d_visualizer.html` can be opened in **any standard web browser** directly from disk with zero setup!

---

## 2. Telemetry Serialization in C++

In [`src/simulation.cpp`](file:///h:/Code/MyOwn/Main/Best/MOP%20Simulator/src/simulation.cpp#L706-L768), C++ constructs JSON arrays by serializing vector frames:

```cpp
std::stringstream dropFramesJson;
dropFramesJson << "[";
for (size_t j = 0; j < r.drop_frames.size(); ++j) {
    const auto& f = r.drop_frames[j];
    dropFramesJson << "{t:" << f.time << ",y:" << f.depth 
                   << ",v:" << f.velocity << ",m:" << f.mach
                   << ",sb:" << (f.is_sonic_boom ? "true" : "false") << "}";
    if (j + 1 < r.drop_frames.size()) dropFramesJson << ",";
}
dropFramesJson << "]";
```

### Key Techniques:
- `std::stringstream`: Efficient string accumulation without repeated reallocation.
- **Ternary Operator (`? :`):** Converts C++ `bool` values (`true`/`false`) directly to JS boolean keywords (`"true"`/`"false"`).
- `escapeJSON()`: Escapes special characters (`"`, `\`, `\n`) to prevent JSON formatting syntax errors or injection bugs.

---

## 3. String Replacement Injection

Inside `assets/visualizer_template.html`, placeholder tokens exist:

```html
<script>
    const scenarios = [
{{SCENARIOS_DATA}}
    ];
</script>
```

In C++, `replaceAll()` finds the placeholder token and replaces it with the serialized telemetry string:

```cpp
void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty()) return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

// Inject buttons and telemetry data into HTML string template
replaceAll(html, "{{SCENARIO_BUTTONS}}", buttons.str());
replaceAll(html, "{{SCENARIOS_DATA}}", data.str());
```

---

## 4. Frontend Rendering Loop (Three.js)

Once the generated HTML file opens in a browser, Three.js parses the injected `scenarios` array:

1. `selectScenario(index)` sets up the target layer boxes (`THREE.BoxGeometry`).
2. `animate()` uses `THREE.Clock` to lerp (linearly interpolate) velocity, altitude, and depth frames smoothly across render ticks.
3. `createCraterGeometry(depth)` dynamically constructs a 3D lathe geometry cone/cylinder tunnel (`THREE.LatheGeometry`) depicting sequential bomb burrowing.

---

## 5. Benefits for C++ Developers

- **No Third-Party Web Server Dependencies:** Users don't need `npm`, `node`, `python`, or `http-server`.
- **Portable Output:** The resulting `.html` file is a single self-contained document that can be emailed or archived.
- **High Performance:** C++ does heavy math calculation on CPU in microseconds; JavaScript handles smooth 60 FPS GPU rendering in Three.js.
