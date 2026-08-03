# C++ Structs vs. Classes for Beginners

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

---

## 1. The Single Difference Between `struct` and `class`

In C++, `struct` and `class` are nearly identical. There is only **ONE** technical difference:

- **`struct`**: Members and base inheritance are **`public` by default**.
- **`class`**: Members and base inheritance are **`private` by default**.

```cpp
struct TargetLayer {
    std::string material_name; // Automatically PUBLIC!
    double thickness;          // Automatically PUBLIC!
};

class ImpactSimulator {
    Projectile proj;           // Automatically PRIVATE!
    Target target;             // Automatically PRIVATE!
public:
    ImpactSimulator(const Projectile& p, const Target& t); // Explicitly PUBLIC
};
```

---

## 2. When to Use `struct` (Data Transfer Objects)

Use a `struct` when your data is a passive collection of variables with no internal invariant protection or hidden internal states.

In MOP Simulator, parameters like target concrete layers, projectile geometry, and telemetry frames are declared as `struct` inside [`include/simulation.hpp`](file:///h:/Code/MyOwn/Main/Best/MOP%20Simulator/include/simulation.hpp#L15-L55):

```cpp
struct TargetLayer {
    std::string material_name = "Concrete";
    double thickness = 1.0;
    double density = 2500.0;
    double compressive_strength = 60.0e6;
    double rebar_volume_fraction = 0.0;
    double rebar_yield_strength = 0.0;
    double pulverized_depth = 0.0;
};
```

### Advantages for Beginners:
1. **Direct Access:** You can read and assign variables directly: `layer.thickness = 15.0;`.
2. **Default Initializers:** Notice how default values (`= 2500.0`) prevent uninitialized memory bugs!

---

## 3. When to Use `class` (Encapsulated Behavior)

Use a `class` when your component encapsulates complex algorithms, internal state management, or methods that operate on private data.

In [`include/simulation.hpp`](file:///h:/Code/MyOwn/Main/Best/MOP%20Simulator/include/simulation.hpp#L60-L80), `ImpactSimulator` is a `class`:

```cpp
class ImpactSimulator {
private:
    Projectile proj;
    Target target;
    PhysicsConstants cons;

    // Internal helper function hidden from outside callers
    double getMachDependentDrag(double mach, double baseCd) const;

public:
    ImpactSimulator(const Projectile& p, const Target& t, const PhysicsConstants& c);

    // Main public method
    SimulationResult simulate(const ImpactScenario& scenario);
};
```

### Advantages of Encapsulation:
1. **Protection:** Outside functions cannot corrupt `proj` or `target` mid-simulation.
2. **Clean Public API:** Callers only need to know about `.simulate()`. The internal drag calculations, Runge-Kutta numerical steps, and thermal ablation logic are safely hidden inside `private`.

---

## 4. Modern C++ Initializers & Best Practices

In modern C++ (C++11 and up):
- Always provide default member initializers inside `struct` definitions (`double thickness = 1.0;`).
- Use `const` on methods that do not modify class members (e.g. `double getMachDependentDrag(...) const`).
