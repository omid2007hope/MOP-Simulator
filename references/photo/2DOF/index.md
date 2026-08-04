This is a highly comprehensive and physically rigorous mathematical framework for simulating unpowered, fin-stabilized flight dynamics. By combining the US Standard Atmosphere, Mach-dependent drag profiles, and 4th-Order Runge-Kutta (RK4) integration, this model provides weapons-grade fidelity for ballistic trajectories and pitch responses.

Here is your model cleanly formatted as a reference guide:

## 1. Atmospheric Model (US Standard Atmosphere 1976)

As altitude $y$ changes, atmospheric properties update continuously:

- **Temperature:**
  $$T(y) = T_0 + L \cdot y$$

(where $T_0 = 288.15\text{ K}$ and $L = -0.0065\text{ K/m}$)

- **Air Density:**
  $$\rho(y) = \frac{P(y) \cdot M_{\text{air}}}{R \cdot T(y)}$$

- **Speed of Sound & Mach Number:**
  $$c(y) = \sqrt{\frac{\gamma \cdot R \cdot T(y)}{M_{\text{air}}}}, \quad M = \frac{v}{c(y)}$$

## 2. Aerodynamic Drag Force ($F_d$)

Drag acts directly opposite to the velocity vector $v = \sqrt{v_x^2 + v_y^2}$:

$$F_d = \frac{1}{2} \cdot \rho(y) \cdot v^2 \cdot C_d(M) \cdot A_{\text{ref}}$$

_(Where $C_d(M)$ is the Mach-dependent drag coefficient and $A_{\text{ref}} = \frac{\pi}{4} d^2$ is the frontal cross-sectional area)._

## 3. 2DOF Translation Dynamics

Decomposing forces into horizontal and vertical net accelerations based on the flight path angle $\gamma = \operatorname{atan2}(v_x, v_y)$:

| Force Component      | Acceleration Equation                                      |
| -------------------- | ---------------------------------------------------------- |
| **Horizontal ($x$)** | $$a_x = -\frac{F_d \cdot \sin(\gamma)}{m_{\text{total}}}$$ |

|
| **Vertical ($y$)** | $$a_y = g - \frac{F_d \cdot \cos(\gamma)}{m_{\text{total}}}$$

|

---

---

## 4. Pitch Rotation & Fin Torque Dynamics ($\theta$)

For fin-stabilized projectiles (like the GBU-57), the tail fins generate torque around the Center of Mass based on the Angle of Attack $\alpha = \theta - \gamma$:

- **Aerodynamic Restoring Moment:**
  $$M_\alpha = -C_{m\alpha} \cdot q \cdot A_{\text{ref}} \cdot d \cdot \alpha$$

- **Pitch Damping Moment:**
  $$M_q = -C_{mq} \cdot q \cdot A_{\text{ref}} \cdot d^2 \cdot \frac{\dot{\theta}}{2v}$$

- **Net Angular Acceleration:**
  $$\ddot{\theta} = \frac{M_\alpha + M_q}{I_z}$$

## 5. 4th-Order Runge-Kutta (RK4) Integration

At each $10\text{ ms}$ time step $\Delta t$, the state array updates simultaneously to prevent the divergence common in simple Euler methods:

$$\begin{aligned} v_x(t+\Delta t) &= v_x(t) + \int a_x \, dt \\ v_y(t+\Delta t) &= v_y(t) + \int a_y \, dt \\ \theta(t+\Delta t) &= \theta(t) + \int \dot{\theta} \, dt \end{aligned}$$

---

To see how aerodynamic drag fundamentally alters a standard parabolic arc in 2DOF, you can experiment with this interactive trajectory simulator:

> **Key insight:** Because dynamic pressure $q = \frac{1}{2} \rho v^2$ scales with velocity squared, a high-altitude drop will initially experience very little drag (due to low $\rho$), allowing it to build immense speed before slamming into the denser lower atmosphere where $F_d$ spikes dramatically.
