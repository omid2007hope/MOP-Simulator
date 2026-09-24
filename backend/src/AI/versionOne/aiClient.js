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
	 * Generic helper to call Gemini API with JSON enforcement and exponential backoff retry.
	 * Retries automatically on transient 503 (high demand) and 429 (rate limit) errors.
	 * @param {number} [maxRetries=4] - Maximum number of retry attempts
	 */
	async _callGemini(promptText, inputData, temperature = 1.0, maxRetries = 4) {
		const key = this._getApiKey();
		let lastError;

		for (let attempt = 1; attempt <= maxRetries; attempt++) {
			try {
				const response = await fetch(`https://generativelanguage.googleapis.com/v1beta/models/gemini-3.6-flash:generateContent`, {
					method: 'POST',
					headers: { 
						'Content-Type': 'application/json',
						'x-goog-api-key': key
					},
					body: JSON.stringify({
						systemInstruction: { parts: [{ text: promptText }] },
						contents: [{ parts: [{ text: JSON.stringify(inputData) }] }],
						generationConfig: { responseMimeType: "application/json", temperature }
					})
				});
				
				const data = await response.json();

				// Retry on transient server-side errors (503 high demand, 429 rate limit)
				if (response.status === 503 || response.status === 429) {
					const delay = Math.pow(2, attempt) * 1000; // 2s, 4s, 8s, 16s
					const msg = data.error ? data.error.message : `HTTP ${response.status}`;
					console.warn(`[AI Client] Attempt ${attempt}/${maxRetries} failed (${msg}). Retrying in ${delay / 1000}s...`);
					await new Promise(r => setTimeout(r, delay));
					lastError = new Error(msg);
					continue;
				}

				if (!response.ok || data.error) {
					throw new Error(data.error ? data.error.message : `HTTP Error ${response.status}`);
				}

				if (data.candidates && data.candidates[0].content) {
					const rawText = data.candidates[0].content.parts[0].text;
					
					// Try to extract JSON from a markdown code block first
					const match = rawText.match(/```(?:json)?\s*([\s\S]*?)\s*```/i);
					let jsonStr = match ? match[1] : rawText;
					
					// Strip any conversational padding outside the outermost object/array
					const objStart = jsonStr.indexOf('{');
					const objEnd = jsonStr.lastIndexOf('}');
					const arrStart = jsonStr.indexOf('[');
					const arrEnd = jsonStr.lastIndexOf(']');
					
					if (objStart !== -1 && objEnd !== -1 && (arrStart === -1 || objStart < arrStart)) {
						jsonStr = jsonStr.substring(objStart, objEnd + 1);
					} else if (arrStart !== -1 && arrEnd !== -1) {
						jsonStr = jsonStr.substring(arrStart, arrEnd + 1);
					}
					
					return JSON.parse(jsonStr);
				} else {
					throw new Error(`Gemini returned unexpected format: ${JSON.stringify(data)}`);
				}
			} catch (e) {
				// Only retry on transient errors; rethrow permanent ones immediately
				if (attempt < maxRetries && (e.message.includes('high demand') || e.message.includes('rate limit'))) {
					const delay = Math.pow(2, attempt) * 1000;
					console.warn(`[AI Client] Attempt ${attempt}/${maxRetries} failed. Retrying in ${delay / 1000}s...`);
					await new Promise(r => setTimeout(r, delay));
					lastError = e;
					continue;
				}
				console.error("[AI Client] Gemini API call failed.", e.message);
				throw e;
			}
		}

		console.error("[AI Client] All retry attempts exhausted.", lastError.message);
		throw lastError;
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
