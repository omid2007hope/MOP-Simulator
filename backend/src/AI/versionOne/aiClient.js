// © 2026 Omid Teimory. All rights reserved.
const { researchConductor, researchAnalyst, articleWriter } = require('./Prompt');

class AIClient {
	/**
	 * Validates the presence of the Gemini API key.
	 * @throws {Error} if the key is missing to prevent silent failures or hardcoding.
	 */
	_getApiKey() {
		const key = process.env.GEMINI_API_KEY;
		if (!key) {
			throw new Error("Safety Trigger: GEMINI_API_KEY is missing. Hardcoded mock data is strictly prohibited.");
		}
		return key;
	}

	/**
	 * Generic helper to call Gemini API with JSON enforcement.
	 */
	async _callGemini(promptText, inputData, temperature = 1.0) {
		const key = this._getApiKey();
		try {
			const response = await fetch(`https://generativelanguage.googleapis.com/v1beta/models/gemini-3.6-flash:generateContent?key=${key}`, {
				method: 'POST',
				headers: { 'Content-Type': 'application/json' },
				body: JSON.stringify({
					systemInstruction: { parts: [{ text: promptText }] },
					contents: [{ parts: [{ text: JSON.stringify(inputData) }] }],
					generationConfig: { responseMimeType: "application/json", temperature }
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

	/**
	 * Generates a C++ simulation config based on the research topic.
	 * @param {Object} researchData - { title, description, count }
	 * @param {Number} currentCycle - The current simulation cycle number (1-indexed)
	 * @param {Number} totalCycles - The total number of cycles requested
	 * @returns {Object} Simulation config (Projectile, Target, Scenarios, etc.)
	 */
	async generateScenario(researchData, currentCycle = 1, totalCycles = 1) {
		console.log(`[AI Client] Generating scenario for topic: ${researchData.title} (Cycle ${currentCycle}/${totalCycles})`);
		const input = { ...researchData, currentCycle, totalCycles, random_seed: Math.random() };
		
		// High temperature (1.5) for Monte Carlo parameter sweep exploration
		return this._callGemini(researchConductor, input, 1.5);
	}

	/**
	 * Analyzes raw simulation results to extract physics insights.
	 * @param {string} title - Research title
	 * @param {string} description - Research description/hypothesis
	 * @param {Object} stats - Aggregate statistics from all simulation runs
	 * @param {Array} sampleResults - A sample of raw result documents for context
	 * @returns {Object} { insights, anomalies, conclusions }
	 */
	async analyzeResults(title, description, stats, sampleResults) {
		console.log(`[AI Client] Analyzing simulation results for: "${title}"`);
		const input = { title, description, stats, sampleResults };
		
		// Lower temperature (0.7) for highly precise, analytical deductions
		return this._callGemini(researchAnalyst, input, 0.7);
	}

	/**
	 * Synthesizes research analysis into a formatted scientific article.
	 * @param {string} title - Research title
	 * @param {string} description - Research description/hypothesis
	 * @param {Object} stats - Aggregate statistics
	 * @param {Object} analysis - The insights extracted by the researchAnalyst step
	 * @param {Array} sampleResults - A sample of raw result documents for context
	 * @returns {Object} { abstract, content, key_findings }
	 */
	async generateArticle(title, description, stats, analysis, sampleResults) {
		console.log(`[AI Client] Generating research article for: "${title}"`);
		const input = { title, description, stats, analysis, sampleResults };
		
		// Standard temperature (1.0) for creative but grounded synthesis
		return this._callGemini(articleWriter, input, 1.0);
	}
}

module.exports = new AIClient();
