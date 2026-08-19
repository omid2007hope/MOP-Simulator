const researchPrompt = require('./Prompt');

class AIClient {
    /**
     * Sends the research topic to the AI and requests a generated simulation configuration.
     * @param {Object} researchData - Contains the research title, description, and cycle count.
     * @returns {Object} - A parsed JSON object containing the Projectile, Target, and Scenario configs.
     */
    async generateScenario(researchData) {
        // TODO: Integrate actual LLM SDK (OpenAI/Gemini) here.
        // The prompt asks the AI to generate parameters based on `researchPrompt`.
        
        console.log(`[AI Client] Generating scenario for topic: ${researchData.title}`);

        // Mocked response simulating the AI's structured JSON output
        return {
            "Simulation": {
                "choice": 3,
                "numBombs": 2,
                "tos_agree": true
            },
            "Projectile": {
                "name": "AI Generated MOP Variant",
                "length": 6.5,
                "diameter": 0.85,
                "curvature_noseReduce": 4.5,
                "total_mass": 14000.0,
                "explosive_mass": 2500.0,
                "explosive_energy_j_per_kg": 5.2e6,
                "casing_density": 7850.0,
                "yield_strength": 2.5e9,
                "area_moment_inertia": 0.025,
                "elastic_modulus": 200.0e9,
                "casing_wall_thickness": 0.12,
                "hugoniot_c0": 4570.0,
                "hugoniot_s": 1.49,
                "explosive_critical_energy": 3.0e15,
                "specific_heat": 460.0,
                "melting_point": 1800.0,
                "heat_of_fusion": 272000.0
            },
            "Target": {
                "name": "AI Hypothesized Deep Bunker",
                "layers": [
                    {
                        "material_name": "Reinforced Concrete",
                        "thickness": 8.0,
                        "density": 2650.0,
                        "compressive_strength": 70.0e6,
                        "rebar_volume_fraction": 0.04,
                        "rebar_yield_strength": 500.0e6,
                        "pulverized_depth": 0.0,
                        "hugoniot_c0": 3200.0,
                        "hugoniot_s": 1.9
                    }
                ]
            },
            "Scenario": {
                "numScenarios": 1,
                "name": "Deep Penetration Test",
                "altitude_ft": 45000.0,
                "velocity": 260.0,
                "obliquity_angle": 0.0,
                "flight_path_angle": 355.0,
                "angle_of_attack": 0.0
            }
        };
    }
}

module.exports = new AIClient();
