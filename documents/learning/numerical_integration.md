# Numerical Integration: Runge-Kutta 4th Order (RK4) in C++

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

---

## 1. What is Numerical Integration?

In high school physics, you learn basic equations of motion like:

$$\text{Distance} = \text{Velocity} \times \text{Time}$$

However, in real-world terminal ballistics, **velocity is constantly changing** due to aerodynamic drag in the atmosphere and extreme resistive forces inside concrete!

Because drag changes with altitude, density, and speed, we cannot solve the exact velocity with a simple formula. Instead, we use **Numerical Integration** to step through time in tiny fractions of a second ($\Delta t = 0.01\text{ s}$).

---

## 2. Euler's Method vs. Runge-Kutta 4th Order (RK4)

### Euler's Method (Simple but Inaccurate)
Euler's method calculates acceleration at the start of a time step and assumes it stays constant throughout the step:

$$v_{new} = v_{old} + a \cdot \Delta t$$

- **Problem:** Small errors compound rapidly. In hypervelocity impact simulations, Euler's method will cause the projectile to "blow up" or stop unnaturally early.

### Runge-Kutta 4th Order (RK4) — **Used in MOP Simulator**
RK4 samples acceleration at **4 different points** across the time step (start, mid-point trial 1, mid-point trial 2, and end), taking a weighted average of all four slopes!

---

## 3. RK4 Code Breakdown in `simulation.cpp`

Look at [`src/simulation.cpp`](file:///h:/Code/MyOwn/Main/Best/MOP%20Simulator/src/simulation.cpp#L101-L126) for the free-fall atmospheric drop phase:

```cpp
auto calc_accel = [&](double alt_m, double vel) {
    double density = findAirDensityByAltitude(alt_m * 3.28084);
    double mach = vel / cons.SPEED_OF_SOUND;
    double cd = getMachDependentDrag(mach, dragCoefficient);
    double f = 0.5 * density * vel * vel * cd * area;
    return cons.gravity - (f / proj.total_mass); // Net Acceleration
};

// 1. First slope (at beginning of step)
double k1_v = calc_accel(y_m, current_velocity);
double k1_y = -current_velocity;

// 2. Second slope (at half-step using k1)
double k2_v = calc_accel(y_m + 0.5 * dt_drop * k1_y, current_velocity + 0.5 * dt_drop * k1_v);
double k2_y = -(current_velocity + 0.5 * dt_drop * k1_v);

// 3. Third slope (at half-step using k2)
double k3_v = calc_accel(y_m + 0.5 * dt_drop * k2_y, current_velocity + 0.5 * dt_drop * k2_v);
double k3_y = -(current_velocity + 0.5 * dt_drop * k2_v);

// 4. Fourth slope (at full step using k3)
double k4_v = calc_accel(y_m + dt_drop * k3_y, current_velocity + dt_drop * k3_v);
double k4_y = -(current_velocity + dt_drop * k3_v);

// Weighted average update (Simpsons Rule weighting: 1/6 * (k1 + 2*k2 + 2*k3 + k4))
current_velocity += (dt_drop / 6.0) * (k1_v + 2 * k2_v + 2 * k3_v + k4_v);
y_m += (dt_drop / 6.0) * (k1_y + 2 * k2_y + 2 * k3_y + k4_y);
```

---

## 4. Why Use Lambda Functions (`[&]`) in C++?

Notice the `calc_accel` definition above:

```cpp
auto calc_accel = [&](double alt_m, double vel) { ... };
```

This is a **C++ Lambda Function** (an anonymous inline function).
- The `[&]` capture clause tells C++ to allow the lambda function to access variables from the outer scope (`cons`, `proj`, `dragCoefficient`) by reference without passing them as arguments every time.
- It keeps physics formulas clean, fast, and organized right where they are used.
