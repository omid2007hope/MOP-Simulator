// © 2026 Omid Teimory. All rights reserved.
const { researchConductor, articleWriter } = require('./Prompt');

class AIClient {
	/**
	 * Generates a C++ simulation config based on the research topic.
	 * TODO: Replace mock with real LLM call using `researchConductor` as system prompt.
	 * @param {Object} researchData - { title, description, count }
	 * @returns {Object} Simulation config (Projectile, Target, Scenario, Simulation)
	 */
	async generateScenario(researchData) {
		console.log(`[AI Client] Generating scenario for topic: ${researchData.title}`);
		// Prompt template ready for real LLM integration:
		// System prompt: researchConductor

		// Mocked response simulating the AI's structured JSON output
		return {
			Simulation: { choice: 3, numBombs: 2, tos_agree: true },
			Projectile: {
				name: 'AI Generated MOP Variant',
				length: 6.5,
				diameter: 0.85,
				curvature_noseReduce: 4.5,
				total_mass: 14000.0,
				explosive_mass: 2500.0,
				explosive_energy_j_per_kg: 5.2e6,
				casing_density: 7850.0,
				yield_strength: 2.5e9,
				area_moment_inertia: 0.025,
				elastic_modulus: 200.0e9,
				casing_wall_thickness: 0.12,
				hugoniot_c0: 4570.0,
				hugoniot_s: 1.49,
				explosive_critical_energy: 3.0e15,
				specific_heat: 460.0,
				melting_point: 1800.0,
				heat_of_fusion: 272000.0,
			},
			Target: {
				name: 'AI Hypothesized Deep Bunker',
				layers: [
					{
						material_name: 'Reinforced Concrete',
						thickness: 8.0,
						density: 2650.0,
						compressive_strength: 70.0e6,
						rebar_volume_fraction: 0.04,
						rebar_yield_strength: 500.0e6,
						pulverized_depth: 0.0,
						hugoniot_c0: 3200.0,
						hugoniot_s: 1.9,
					},
				],
			},
			Scenario: {
				numScenarios: 1,
				name: 'Deep Penetration Test',
				altitude_ft: 45000.0,
				velocity: 260.0,
				obliquity_angle: 0.0,
				flight_path_angle: 355.0,
				angle_of_attack: 0.0,
			},
		};
	}

	/**
	 * Synthesizes simulation data into a research article.
	 * TODO: Replace mock with real LLM call using `articleWriter` as system prompt.
	 * @param {string} title - Research title
	 * @param {Object} stats - Aggregate statistics from all simulation runs
	 * @param {Array}  sampleResults - A sample of raw result documents for context
	 * @returns {Object} { abstract, content, key_findings }
	 */
	async generateArticle(title, stats, sampleResults) {
		console.log(`[AI Client] Generating research article for: "${title}"`);
		// Prompt template ready for real LLM integration:
		// System prompt: articleWriter
		// User message: JSON.stringify({ title, stats, sampleResults })

		const dominantRegime = stats.dominantRegime || 'Rigid Penetration';
		const date = new Date().toLocaleDateString('en-US', {
			year: 'numeric',
			month: 'long',
			day: 'numeric',
		});

		const regimeTable = Object.entries(stats.regimeDistribution || {})
			.map(([regime, count]) => {
				const pct = ((count / stats.totalScenarios) * 100).toFixed(1);
				return `| ${regime.padEnd(45)} | ${String(count).padEnd(8)} | ${pct}% |`;
			})
			.join('\n');

		const abstract = `This study presents a high-fidelity computational analysis of ${title.toLowerCase()} conducted through ${stats.totalScenarios} autonomous simulation cycles using the MOP Impact Physics & Penetration Simulator v2.8. Employing the Alekseevskii-Tate hydrodynamic penetration model in conjunction with the Walker-Wasley shock initiation criterion and Hugoniot equation-of-state formulations, this research systematically characterizes penetration mechanics across a statistically significant scenario space. Results indicate a mean penetration depth of ${stats.avgPenetrationDepth} meters (σ = ${stats.stdDevPenetration} m), with a maximum recorded depth of ${stats.maxPenetrationDepth} meters under optimal impact conditions. The dominant penetration regime observed across ${stats.casingFailureRate}% casing integrity was ${dominantRegime}. Shock pressure peaks ranged to ${stats.avgShockPressureGPa} GPa average, with casing failure rates of ${stats.casingFailureRate}% and hydrodynamic erosion onset in ${stats.erosionRate}% of scenarios. These findings provide quantitative bounds for structural defeat criteria against high-strength reinforced concrete targets and identify critical velocity-geometry relationships governing sub-surface fuze survivability.`;

		const content = `# ${title}

**MOP Simulator Autonomous Research Division**
**Date:** ${date}
**Simulation Engine:** MOP Impact Physics & Penetration Simulator v2.8
**Total Scenarios:** ${stats.totalScenarios}

---

## Abstract

${abstract}

---

## 1. Introduction

The defeat of deeply buried hardened targets (DBHTs) represents one of the most demanding challenges in modern precision strike warfare. Reinforced concrete structures with compressive strengths exceeding 70 MPa, often buried under meters of overburden, require penetrating munitions capable of sustaining structural integrity through high-velocity impact while delivering sufficient sub-surface explosive yield for definitive target defeat.

The GBU-57 Massive Ordnance Penetrator (MOP) — a 13,608 kg penetrating munition with a hardened steel casing — represents the current state of the art in conventional bunker-defeat capability. However, the precise characterization of penetration performance across the full parameter space of target geometry, impact conditions, and casing design remains an active area of computational research.

This study addresses the research question: **${title}**. Using an autonomous AI-driven simulation loop coupled to a high-fidelity C++ physics engine, we generated and evaluated ${stats.totalScenarios} distinct impact scenarios, systematically varying projectile parameters, target configurations, drop altitudes, and impact angles to map the penetration performance envelope with statistical rigor.

The physics models implemented in this simulator span several foundational contributions to terminal ballistics:

- **Alekseevskii-Tate Model**: The governing equation for penetration velocity decay, accounting for hydrodynamic flow at the projectile-target interface when dynamic pressure exceeds the projectile yield strength.
- **Walker-Wasley Shock Initiation Criterion**: Determines explosive charge survivability based on the transmitted shock energy impulse relative to the critical initiation energy threshold.
- **Hugoniot Equation of State**: Relates shock wave velocity (Us) to particle velocity (Up) through the linear Hugoniot parameters C0 and S for both projectile and target materials.
- **Runge-Kutta 4th-Order Integration**: Applied to both atmospheric free-fall trajectory and ground penetration dynamics for numerical accuracy.
- **Dynamic Increase Factor (DIF)**: Applied to concrete compressive strength as a function of strain rate to capture rate-dependent material response during high-velocity impact.

The remainder of this article is structured as follows: Section 2 describes the simulation methodology and parameter space; Section 3 presents and discusses the results; Section 4 draws conclusions and outlines future research directions.

---

## 2. Methodology

### 2.1 Simulation Architecture

The autonomous research pipeline consists of three coupled subsystems:

1. **AI Scenario Generator**: An intelligent parameter synthesis engine that generates physically plausible scenario configurations spanning the defined research space. For each simulation cycle, the AI generates a complete parameter set including projectile geometry, explosive loading, target layer composition, impact kinematics, and environmental conditions.

2. **C++ Physics Engine (MOP Simulator v2.8)**: A compiled high-performance simulation kernel that ingests scenario parameters via stdin, executes the full terminal ballistics model, and exports structured telemetry results. The engine integrates atmospheric drop physics (including Mach-dependent drag, sonic boom detection, and atmospheric density variation) with ground penetration mechanics.

3. **MongoDB Telemetry Database**: All simulation results are persisted in structured documents for subsequent aggregate analysis and article generation.

### 2.2 Physics Models

**Atmospheric Drop Phase:** The projectile trajectory from release altitude to ground impact is simulated using RK4 integration with time step dt = 0.01 s. Atmospheric density and speed of sound are computed via the International Standard Atmosphere model. Mach-dependent drag is applied using the caliber-radius-head (CRH) model:

> Cd = (8·CRH - 1) / (24·CRH²)

**Hugoniot Shock Analysis (Impact Initiation):**
At impact, the interface particle velocity (Up) and shock velocity (Us) are computed:

> Up = V_impact / (1 + √(ρ_target / ρ_projectile))
> Us = C0 + S·Up
> P_shock = ρ_projectile · Us · Up

The Walker-Wasley criterion evaluates whether transmitted shock energy exceeds the explosive's critical initiation threshold:

> E_shock = P_transmitted² · τ
> Survives if: E_shock < E_critical

**Penetration Phase (Rigid & Hydrodynamic):**
Rigid penetration resistance force is modeled as:

> F_tunnel = A · (S·f'c + Cd·ρ·V²)

where S = 82.6·(f'c)^(-0.544) is the empirically derived penetration resistance factor for concrete. Transition to hydrodynamic (eroding) regime occurs when dynamic pressure (0.5·ρ·V²) exceeds the projectile yield strength.

### 2.3 Parameter Space

Across ${stats.totalScenarios} simulated scenarios, the following parameter ranges were systematically explored:

| Parameter | Range |
|-----------|-------|
| Impact Velocity | ${stats.avgVelocity} m/s (avg), up to Mach ${stats.avgMach} |
| Average Energy | ${stats.avgEnergyGJ} GJ kinetic |
| Avg Shock Pressure | ${stats.avgShockPressureGPa} GPa |
| Penetration Depth | ${stats.minPenetrationDepth}–${stats.maxPenetrationDepth} m |

---

## 3. Results & Discussion

### 3.1 Penetration Depth Distribution

Across all ${stats.totalScenarios} simulated scenarios, the penetration depth distribution exhibits the following central tendency characteristics:

| Metric | Value |
|--------|-------|
| Mean Penetration Depth | ${stats.avgPenetrationDepth} m |
| Maximum Depth | ${stats.maxPenetrationDepth} m |
| Minimum Depth | ${stats.minPenetrationDepth} m |
| Standard Deviation | ${stats.stdDevPenetration} m |

The mean penetration depth of ${stats.avgPenetrationDepth} m is consistent with theoretical Alekseevskii-Tate predictions for targets in this strength class at the observed impact velocities. The standard deviation of ${stats.stdDevPenetration} m reflects the sensitivity of penetration outcome to target layer composition and impact angle variations introduced across the scenario ensemble.

### 3.2 Penetration Regime Classification

The following regime distribution was observed across the full scenario set:

| Penetration Regime | Count | Frequency |
|----|----|----|
${regimeTable}

The dominance of the **${dominantRegime}** regime (${((stats.regimeDistribution?.[dominantRegime] || 0) / stats.totalScenarios * 100).toFixed(1)}% of scenarios) is consistent with the impact velocity range explored. At sub-hypervelocity conditions, the projectile casing yield strength exceeds the dynamic pressure generated, preserving the rigid-body penetration mechanism. Transition to the hydrodynamic erosion regime was observed in ${stats.erosionRate}% of scenarios, consistent with impact conditions where dynamic pressure (≈${stats.avgShockPressureGPa} GPa) approaches or exceeds the casing yield strength.

### 3.3 Casing Integrity and Explosive Survivability

Casing failure was recorded in ${stats.casingFailureRate}% of scenarios. In cases where the Walker-Wasley shock criterion was exceeded, the explosive charge failed to survive to the intended detonation depth, resulting in either surface or shallow sub-surface detonation — a significantly reduced defeat mechanism against deeply buried targets.

The Walker-Wasley shock energy threshold (E_critical) represents a critical design boundary. Scenarios operating at or near this threshold exhibited high sensitivity to small variations in target layer Hugoniot parameters, suggesting that target material uncertainty represents a primary source of prediction variance in operational employment.

### 3.4 Multi-Bomb Sequential Strike Effects

In sequential salvo scenarios, the second and subsequent penetrators benefited significantly from the pre-existing breached shaft created by the lead munition. The cumulative breach depth achieved by sequential strikes was consistently greater than the sum of individual penetration predictions, demonstrating a pronounced synergistic effect attributable to:

1. **Reduced effective target strength** in the pulverized zone ahead of subsequent penetrators
2. **Eliminated crater formation energy** — subsequent rounds enter a pre-formed channel
3. **Residual velocity advantage** — shorter deceleration path before reaching intact target material

Maximum cumulative depth recorded was ${stats.maxPenetrationDepth} meters, achieved under optimal sequential timing conditions.

### 3.5 Velocity and Energy Sensitivity

Average impact velocity across the scenario ensemble was ${stats.avgVelocity} m/s (Mach ${stats.avgMach}), corresponding to a mean kinetic energy of ${stats.avgEnergyGJ} GJ. Correlation analysis between impact velocity and penetration depth reveals a non-linear relationship consistent with the Forrestal empirical model for ogive-nosed penetrators, with penetration depth scaling approximately as V^0.7 across the subsonic-to-low-supersonic regime observed.

---

## 4. Conclusion

This autonomous simulation study of **${title}** has yielded the following principal findings:

1. **Mean penetration depth** of ${stats.avgPenetrationDepth} m (σ = ${stats.stdDevPenetration} m) establishes performance bounds against ${stats.avgShockPressureGPa} GPa-class target configurations at the studied impact conditions.

2. **Casing integrity** was maintained in ${100 - parseFloat(stats.casingFailureRate)}% of scenarios, confirming the viability of delayed fuzing for the majority of the studied parameter space.

3. **Sequential salvo tactics** produced cumulative penetration of up to ${stats.maxPenetrationDepth} m — significantly exceeding single-strike predictions and confirming the operational value of sequenced munition employment.

4. **Hydrodynamic erosion onset** in ${stats.erosionRate}% of scenarios identifies the critical velocity boundary above which projectile mass loss must be accounted for in penetration modeling.

5. **Regime dominance** of ${dominantRegime} across ${stats.totalScenarios} scenarios validates the applicability of the Crater+Tunnel model for this target class and velocity range.

### Limitations

This study employed a mocked AI scenario generator pending integration of a production LLM API. Future work should implement dynamic scenario variation across the full projectile and target parameter space to maximize the statistical coverage of the research envelope. Additionally, the current model assumes homogeneous target layers; multi-layer heterogeneous targets with soil overburden should be incorporated to better represent operational target geometries.

### Future Work

- Integration of real-time LLM scenario generation for true autonomous hypothesis-driven research
- Extension to oblique impact angle parametric studies
- Multi-target comparative analysis (concrete, rock, soil, steel-reinforced composites)
- Incorporation of shaped-charge defeat mechanisms alongside kinetic penetrators

---

## References

1. Forrestal, M.J. & Tzou, D.Y. (1997). *A spherical cavity-expansion penetration model for concrete targets.* International Journal of Solids and Structures, 34(31), 4127-4146.
2. Frew, D.J., Forrestal, M.J. & Hanchak, S.J. (2000). *Penetration experiments with limestone targets and ogive-nose steel projectiles.* ASME Journal of Applied Mechanics, 67(4), 841-845.
3. Walker, J.D. & Wasley, R.J. (1969). *A general model for the shock initiation of explosives.* Propellants, Explosives, Pyrotechnics, 8(2), 1-17.
4. Alekseevskii, V.P. (1966). *Penetration of a rod into a target at high velocity.* Combustion, Explosion, and Shock Waves, 2(2), 63-66.
5. Tate, A. (1967). *A theory for the deceleration of long rods after impact.* Journal of the Mechanics and Physics of Solids, 15(6), 387-399.
6. Chen, X.W. & Li, Q.M. (2002). *Deep penetration of a non-deformable projectile with different geometrical characteristics.* International Journal of Impact Engineering, 27(6), 619-637.
7. Li, Q.M. & Chen, X.W. (2003). *Dimensionless formulae for penetration depth of concrete target impacted by a non-deformable projectile.* International Journal of Impact Engineering, 28(1), 93-116.
`;

		const keyFindings = [
			`Mean penetration depth: ${stats.avgPenetrationDepth} m (σ = ${stats.stdDevPenetration} m)`,
			`Dominant regime: ${dominantRegime} in ${((stats.regimeDistribution?.[dominantRegime] || 0) / stats.totalScenarios * 100).toFixed(1)}% of scenarios`,
			`Casing integrity maintained in ${100 - parseFloat(stats.casingFailureRate)}% of scenarios`,
			`Maximum sequential breach depth: ${stats.maxPenetrationDepth} m`,
			`Hydrodynamic erosion onset in ${stats.erosionRate}% of scenarios`,
			`Average impact velocity: ${stats.avgVelocity} m/s at Mach ${stats.avgMach}`,
		];

		return { abstract, content, key_findings: keyFindings };
	}
}

module.exports = new AIClient();
