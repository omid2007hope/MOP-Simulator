# JSON Parsing & File I/O in Modern C++ (`nlohmann::json`)

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

---

## 1. Why Use JSON in C++?

C++ programs frequently need to load external configuration settings or data targets without recompiling code every time a parameter changes.

**JSON (JavaScript Object Notation)** is human-readable and standard across the web.

In the MOP Simulator, concrete target properties and bomb geometries are stored inside JSON files in the [`data/`](file:///h:/Code/MyOwn/Main/Best/MOP%20Simulator/data) directory:
- `data/targets.json`: Stores compressive strength, density, rebar fractions, and multi-layer soil/rock thicknesses.
- `data/projectiles.json`: Stores length, mass, casing density, yield strength, and specific heat capacities.

---

## 2. Using `nlohmann::json` Header-Only Library

The project incorporates `nlohmann/json` (a modern header-only C++ library).

In [`src/config_loader.cpp`](file:///h:/Code/MyOwn/Main/Best/MOP%20Simulator/src/config_loader.cpp#L6-L8):

```cpp
#include "nlohmann/json.hpp"
using json = nlohmann::json;
```

---

## 3. Safe Parsing and Default Fallbacks (`item.value()`)

One of the biggest mistakes beginners make in file parsing is assuming every JSON file will always contain valid keys. If a key is missing, standard access (`item["key"]`) can throw an exception or crash the program!

The MOP Simulator uses the safe `.value("key", default_value)` method in [`src/config_loader.cpp`](file:///h:/Code/MyOwn/Main/Best/MOP%20Simulator/src/config_loader.cpp#L24-L37):

```cpp
for (const auto& item : j) {
    Target t;
    t.name = item.value("name", "Unknown Target");

    if (item.contains("layers") && item["layers"].is_array()) {
        for (const auto& l : item["layers"]) {
            TargetLayer layer;
            layer.material_name = l.value("material_name", "Unknown Material");
            layer.thickness = l.value("thickness", 1.0);
            layer.density = l.value("density", 2500.0);
            layer.compressive_strength = l.value("compressive_strength", 60.0e6);
            layer.rebar_volume_fraction = l.value("rebar_volume_fraction", 0.0);
            layer.rebar_yield_strength = l.value("rebar_yield_strength", 0.0);
            t.layers.push_back(layer);
        }
    }
    targets.push_back(t);
}
```

### Why `.value()` is Safe:
If `l.value("rebar_volume_fraction", 0.0)` does not exist in the JSON file, `nlohmann::json` automatically assigns the default fallback value `0.0` instead of crashing!

---

## 4. Using `std::optional` for Safe Lookups

In C++, searching for an object in a container might fail if the name doesn't match anything. Historically, C++ programmers used raw null pointers (`Target*`).

Modern C++ provides `std::optional<T>` in [`include/config_loader.hpp`](file:///h:/Code/MyOwn/Main/Best/MOP%20Simulator/include/config_loader.hpp#L20-L26):

```cpp
std::optional<Target> ConfigLoader::getTargetByName(const std::vector<Target>& targets,
                                                    const std::string& name)
{
    for (const auto& t : targets) {
        if (t.name == name) {
            return t; // Return target wrapped inside std::optional
        }
    }
    return std::nullopt; // Safely signifies "Not Found"
}
```

### Unwrapping `std::optional` in `main.cpp`:
In [`src/main.cpp`](file:///h:/Code/MyOwn/Main/Best/MOP%20Simulator/src/main.cpp#L67-L70):

```cpp
Target object = CONCRETE_DEFAULT; // Fallback default
if (auto t = ConfigLoader::getTargetByName(targetsDb, "High-Quality Hardened Concrete")) {
    object = *t; // Dereference operator '*' unwraps the target if found!
}
```
This pattern eliminates NullPointer exceptions entirely!
