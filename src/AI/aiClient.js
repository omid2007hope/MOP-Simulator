// © 2026 Omid Teimory. All rights reserved.
const { researchConductor, articleWriter } = require('./Prompt');

class AIClient {
	/**
	 * Generates a C++ simulation config based on the research topic.
	 * Calls Gemini API if GEMINI_API_KEY is present, otherwise falls back to a deterministic mock.
	 * @param {Object} researchData - { title, description, count }
	 * @param {Number} currentCycle - The current simulation cycle number (1-indexed)
	 * @param {Number} totalCycles - The total number of cycles requested
	 * @returns {Object} Simulation config (Projectile, Target, Scenario, Simulation)
	 */
	async generateScenario(researchData, currentCycle = 1, totalCycles = 1) {
		console.log(`[AI Client] Generating scenario for topic: ${researchData.title} (Cycle ${currentCycle}/${totalCycles})`);
		
		const GEMINI_API_KEY = process.env.GEMINI_API_KEY;
		if (GEMINI_API_KEY) {
			try {
				const response = await fetch(`https://generativelanguage.googleapis.com/v1beta/models/gemini-3.6-flash:generateContent?key=${GEMINI_API_KEY}`, {
					method: 'POST',
					headers: { 'Content-Type': 'application/json' },
					body: JSON.stringify({
						systemInstruction: { parts: [{ text: researchConductor }] },
						contents: [{ parts: [{ text: JSON.stringify({ ...researchData, currentCycle, totalCycles, random_seed: Math.random() }) }] }],
						generationConfig: { responseMimeType: "application/json", temperature: 1.5 }
					})
				});
				const data = await response.json();
				if (!response.ok || data.error) {
					throw new Error(data.error ? data.error.message : `HTTP Error ${response.status}`);
				}
				if (data.candidates && data.candidates[0].content) {
					const rawText = data.candidates[0].content.parts[0].text.replace(/^```(?:json)?\n?/i, '').replace(/\n?```$/i, '');
					return JSON.parse(rawText);
				} else {
					throw new Error(`Gemini returned unexpected format: ${JSON.stringify(data)}`);
				}
			} catch (e) {
				console.error("[AI Client] Gemini API call failed.", e.message);
				throw e;
			}
		} else {
			console.log("[AI Client] No GEMINI_API_KEY found in environment. Using fallback deterministic mock.");
		}

		// Mocked fallback response simulating the AI's structured JSON output
		return {
			Simulation: { choice: 1, numBombs: 2, tos_agree: true },
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
	 * Calls Gemini API if GEMINI_API_KEY is present, otherwise falls back to a deterministic mock.
	 * @param {string} title - Research title
	 * @param {string} description - Research description/hypothesis
	 * @param {Object} stats - Aggregate statistics from all simulation runs
	 * @param {Array}  sampleResults - A sample of raw result documents for context
	 * @returns {Object} { abstract, content, key_findings }
	 */
	async generateArticle(title, description, stats, sampleResults) {
		console.log(`[AI Client] Generating research article for: "${title}"`);
		
		const GEMINI_API_KEY = process.env.GEMINI_API_KEY;
		if (GEMINI_API_KEY) {
			try {
				const response = await fetch(`https://generativelanguage.googleapis.com/v1beta/models/gemini-3.6-flash:generateContent?key=${GEMINI_API_KEY}`, {
					method: 'POST',
					headers: { 'Content-Type': 'application/json' },
					body: JSON.stringify({
						systemInstruction: { parts: [{ text: articleWriter }] },
						contents: [{ parts: [{ text: JSON.stringify({ title, description, stats, sampleResults }) }] }],
						generationConfig: { responseMimeType: "application/json" }
					})
				});
				const data = await response.json();
				if (!response.ok || data.error) {
					throw new Error(data.error ? data.error.message : `HTTP Error ${response.status}`);
				}
				if (data.candidates && data.candidates[0].content) {
					const rawText = data.candidates[0].content.parts[0].text.replace(/^```(?:json)?\n?/i, '').replace(/\n?```$/i, '');
					return JSON.parse(rawText);
				} else {
					throw new Error(`Gemini returned unexpected format: ${JSON.stringify(data)}`);
				}
			} catch (e) {
				console.error("[AI Client] Gemini API call failed.", e.message);
				throw e;
			}
		}

		// Fallback Mock
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

		const content = `# ${title}\n\n**MOP Simulator Autonomous Research Division**\n**Date:** ${date}\n**Simulation Engine:** MOP Impact Physics & Penetration Simulator v2.8\n**Total Scenarios:** ${stats.totalScenarios}\n\n---\n\n## Abstract\n\n${abstract}\n\n---\n\n## 1. Introduction\n\nThe defeat of deeply buried hardened targets (DBHTs) represents one of the most demanding challenges in modern precision strike warfare. Reinforced concrete structures with compressive strengths exceeding 70 MPa, often buried under meters of overburden, require penetrating munitions capable of sustaining structural integrity through high-velocity impact while delivering sufficient sub-surface explosive yield for definitive target defeat.\n\nThe GBU-57 Massive Ordnance Penetrator (MOP) — a 13,608 kg penetrating munition with a hardened steel casing — represents the current state of the art in conventional bunker-defeat capability. However, the precise characterization of penetration performance across the full parameter space of target geometry, impact conditions, and casing design remains an active area of computational research.\n\nThis study addresses the research question: **${title}**. Using an autonomous AI-driven simulation loop coupled to a high-fidelity C++ physics engine, we generated and evaluated ${stats.totalScenarios} distinct impact scenarios, systematically varying projectile parameters, target configurations, drop altitudes, and impact angles to map the penetration performance envelope with statistical rigor.\n\nThe physics models implemented in this simulator span several foundational contributions to terminal ballistics:\n\n- **Alekseevskii-Tate Model**: The governing equation for penetration velocity decay, accounting for hydrodynamic flow at the projectile-target interface when dynamic pressure exceeds the projectile yield strength.\n- **Walker-Wasley Shock Initiation Criterion**: Determines explosive charge survivability based on the transmitted shock energy impulse relative to the critical initiation energy threshold.\n- **Hugoniot Equation of State**: Relates shock wave velocity (Us) to particle velocity (Up) through the linear Hugoniot parameters C0 and S for both projectile and target materials.\n- **Runge-Kutta 4th-Order Integration**: Applied to both atmospheric free-fall trajectory and ground penetration dynamics for numerical accuracy.\n- **Dynamic Increase Factor (DIF)**: Applied to concrete compressive strength as a function of strain rate to capture rate-dependent material response during high-velocity impact.\n\nThe remainder of this article is structured as follows: Section 2 describes the simulation methodology and parameter space; Section 3 presents and discusses the results; Section 4 draws conclusions and outlines future research directions.\n\n---\n\n## 2. Methodology\n\n### 2.1 Simulation Architecture\n\nThe autonomous research pipeline consists of three coupled subsystems:\n\n1. **AI Scenario Generator**: An intelligent parameter synthesis engine that generates physically plausible scenario configurations spanning the defined research space. For each simulation cycle, the AI generates a complete parameter set including projectile geometry, explosive loading, target layer composition, impact kinematics, and environmental conditions.\n\n2. **C++ Physics Engine (MOP Simulator v2.8)**: A compiled high-performance simulation kernel that ingests scenario parameters via stdin, executes the full terminal ballistics model, and exports structured telemetry results. The engine integrates atmospheric drop physics (including Mach-dependent drag, sonic boom detection, and atmospheric density variation) with ground penetration mechanics.\n\n3. **MongoDB Telemetry Database**: All simulation results are persisted in structured documents for subsequent aggregate analysis and article generation.\n\n### 2.2 Physics Models\n\n**Atmospheric Drop Phase:** The projectile trajectory from release altitude to ground impact is simulated using RK4 integration with time step dt = 0.01 s. Atmospheric density and speed of sound are computed via the International Standard Atmosphere model. Mach-dependent drag is applied using the caliber-radius-head (CRH) model:\n\n> Cd = (8·CRH - 1) / (24·CRH²)\n\n**Hugoniot Shock Analysis (Impact Initiation):**\nAt impact, the interface particle velocity (Up) and shock velocity (Us) are computed:\n\n> Up = V_impact / (1 + √(ρ_target / ρ_projectile))\n> Us = C0 + S·Up\n> P_shock = ρ_projectile · Us · Up\n\nThe Walker-Wasley criterion evaluates whether transmitted shock energy exceeds the explosive's critical initiation threshold:\n\n> E_shock = P_transmitted² · τ\n> Survives if: E_shock < E_critical\n\n**Penetration Phase (Rigid & Hydrodynamic):**\nRigid penetration resistance force is modeled as:\n\n> F_tunnel = A · (S·f'c + Cd·ρ·V²)\n\nwhere S = 82.6·(f'c)^(-0.544) is the empirically derived penetration resistance factor for concrete. Transition to hydrodynamic (eroding) regime occurs when dynamic pressure (0.5·ρ·V²) exceeds the projectile yield strength.\n\n### 2.3 Parameter Space\n\nAcross ${stats.totalScenarios} simulated scenarios, the following parameter ranges were systematically explored:\n\n| Parameter | Range |\n|-----------|-------|\n| Impact Velocity | ${stats.avgVelocity} m/s (avg), up to Mach ${stats.avgMach} |\n| Average Energy | ${stats.avgEnergyGJ} GJ kinetic |\n| Avg Shock Pressure | ${stats.avgShockPressureGPa} GPa |\n| Penetration Depth | ${stats.minPenetrationDepth}–${stats.maxPenetrationDepth} m |\n\n---\n\n## 3. Results & Discussion\n\n### 3.1 Penetration Depth Distribution\n\nAcross all ${stats.totalScenarios} simulated scenarios, the penetration depth distribution exhibits the following central tendency characteristics:\n\n| Metric | Value |\n|--------|-------|\n| Mean Penetration Depth | ${stats.avgPenetrationDepth} m |\n| Maximum Depth | ${stats.maxPenetrationDepth} m |\n| Minimum Depth | ${stats.minPenetrationDepth} m |\n| Standard Deviation | ${stats.stdDevPenetration} m |\n\nThe mean penetration depth of ${stats.avgPenetrationDepth} m is consistent with theoretical Alekseevskii-Tate predictions for targets in this strength class at the observed impact velocities. The standard deviation of ${stats.stdDevPenetration} m reflects the sensitivity of penetration outcome to target layer composition and impact angle variations introduced across the scenario ensemble.\n\n### 3.2 Penetration Regime Classification\n\nThe following regime distribution was observed across the full scenario set:\n\n| Penetration Regime | Count | Frequency |\n|----|----|----|\n${regimeTable}\n\nThe dominance of the **${dominantRegime}** regime (${((stats.regimeDistribution?.[dominantRegime] || 0) / stats.totalScenarios * 100).toFixed(1)}% of scenarios) is consistent with the impact velocity range explored. At sub-hypervelocity conditions, the projectile casing yield strength exceeds the dynamic pressure generated, preserving the rigid-body penetration mechanism. Transition to the hydrodynamic erosion regime was observed in ${stats.erosionRate}% of scenarios, consistent with impact conditions where dynamic pressure (≈${stats.avgShockPressureGPa} GPa) approaches or exceeds the casing yield strength.\n\n### 3.3 Casing Integrity and Explosive Survivability\n\nCasing failure was recorded in ${stats.casingFailureRate}% of scenarios. In cases where the Walker-Wasley shock criterion was exceeded, the explosive charge failed to survive to the intended detonation depth, resulting in either surface or shallow sub-surface detonation — a significantly reduced defeat mechanism against deeply buried targets.\n\nThe Walker-Wasley shock energy threshold (E_critical) represents a critical design boundary. Scenarios operating at or near this threshold exhibited high sensitivity to small variations in target layer Hugoniot parameters, suggesting that target material uncertainty represents a primary source of prediction variance in operational employment.\n\n### 3.4 Multi-Bomb Sequential Strike Effects\n\nIn sequential salvo scenarios, the second and subsequent penetrators benefited significantly from the pre-existing breached shaft created by the lead munition. The cumulative breach depth achieved by sequential strikes was consistently greater than the sum of individual penetration predictions, demonstrating a pronounced synergistic effect attributable to:\n\n1. **Reduced effective target strength** in the pulverized zone ahead of subsequent penetrators\n2. **Eliminated crater formation energy** — subsequent rounds enter a pre-formed channel\n3. **Residual velocity advantage** — shorter deceleration path before reaching intact target material\n\nMaximum cumulative depth recorded was ${stats.maxPenetrationDepth} meters, achieved under optimal sequential timing conditions.\n\n### 3.5 Velocity and Energy Sensitivity\n\nAverage impact velocity across the scenario ensemble was ${stats.avgVelocity} m/s (Mach ${stats.avgMach}), corresponding to a mean kinetic energy of ${stats.avgEnergyGJ} GJ. Correlation analysis between impact velocity and penetration reveals a non-linear relationship consistent with empirical models for ogive-nosed penetrators, scaling approximately as V^0.7 across the observed regime.\n\n---\n\n## 4. Conclusion\n\nThis autonomous simulation study of **${title}** yielded the following findings:\n\n1. **Mean penetration depth** of ${stats.avgPenetrationDepth} m (σ = ${stats.stdDevPenetration} m) establishes performance bounds against ${stats.avgShockPressureGPa} GPa-class targets.\n2. **Casing integrity** was maintained in ${100 - parseFloat(stats.casingFailureRate)}% of scenarios.\n3. **Sequential salvo tactics** produced cumulative penetration of up to ${stats.maxPenetrationDepth} m.\n4. **Regime dominance** of ${dominantRegime} across ${stats.totalScenarios} scenarios validates the applicability of the Crater+Tunnel model.\n`;

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
