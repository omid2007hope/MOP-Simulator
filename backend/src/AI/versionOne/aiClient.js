// © 2026 Omid Teimory. All rights reserved.
const { researchConductor, researchAnalyst } = require('./Prompt');

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
	 * Tries a prioritized list of models. On 503/429, falls back to the next model in the chain.
	 * Non-transient errors (404, bad JSON, etc.) are thrown immediately.
	 */
	async _callGemini(promptText, inputData, temperature = 1.0) {
		const key = this._getApiKey();

		// Ordered fallback chain — confirmed working models first
		const models = [
			'gemini-3.6-flash',
			'gemini-3.5-flash',
			'gemini-3-flash-preview',
		];

		const _parseJson = (rawText) => {
			const match = rawText.match(/```(?:json)?\s*([\s\S]*?)\s*```/i);
			let jsonStr = match ? match[1] : rawText;
			const objStart = jsonStr.indexOf('{');
			const objEnd = jsonStr.lastIndexOf('}');
			const arrStart = jsonStr.indexOf('[');
			const arrEnd = jsonStr.lastIndexOf(']');
			if (objStart !== -1 && objEnd !== -1 && (arrStart === -1 || objStart < arrStart)) {
				jsonStr = jsonStr.substring(objStart, objEnd + 1);
			} else if (arrStart !== -1 && arrEnd !== -1) {
				jsonStr = jsonStr.substring(arrStart, arrEnd + 1);
			}
			// Pass 1: try direct parse — structural newlines in formatted JSON are valid.
			try {
				return JSON.parse(jsonStr);
			} catch (_) {
				// Pass 2: AI embedded literal \n/\r/\t inside a string value (invalid JSON).
				// Only sanitize content between quote pairs, leave structural whitespace intact.
				const sanitized = jsonStr.replace(/"((?:[^"\\]|\\[\s\S])*?)"/g, (_m, c) =>
					'"' + c.replace(/\n/g, '\\n').replace(/\r/g, '\\r').replace(/\t/g, '\\t') + '"'
				);
				return JSON.parse(sanitized);
			}
		};

		const _isTransient = (status, msg) =>
			status === 503 || status === 429 ||
			(msg && (msg.includes('high demand') || msg.includes('rate limit') || msg.includes('UNAVAILABLE')));

		let lastError;
		for (const model of models) {
			// Each model gets 2 attempts before we fall back to the next
			for (let attempt = 1; attempt <= 2; attempt++) {
				try {
					const response = await fetch(
						`https://generativelanguage.googleapis.com/v1beta/models/${model}:generateContent`,
						{
							method: 'POST',
							headers: { 'Content-Type': 'application/json', 'x-goog-api-key': key },
							body: JSON.stringify({
								systemInstruction: { parts: [{ text: promptText }] },
								contents: [{ parts: [{ text: JSON.stringify(inputData) }] }],
								generationConfig: { responseMimeType: 'application/json', temperature }
							})
						}
					);

					const data = await response.json();
					const errMsg = data.error ? data.error.message : null;

					if (_isTransient(response.status, errMsg)) {
						const delay = attempt === 1 ? 2000 : 5000;
						console.warn(`[AI Client] ${model} attempt ${attempt}/2 unavailable. Retrying in ${delay / 1000}s...`);
						await new Promise(r => setTimeout(r, delay));
						lastError = new Error(errMsg || `HTTP ${response.status}`);
						continue; // retry same model once
					}

					if (!response.ok || data.error) {
						// Permanent error (404 deprecated, auth failure, etc.) — skip model immediately
						console.warn(`[AI Client] ${model} failed permanently (${errMsg}). Trying next model...`);
						lastError = new Error(errMsg || `HTTP Error ${response.status}`);
						break; // break inner loop → try next model
					}

					if (data.candidates && data.candidates[0].content) {
						if (model !== 'gemini-3.6-flash') {
							console.log(`[AI Client] Succeeded via fallback model: ${model}`);
						}
						return _parseJson(data.candidates[0].content.parts[0].text);
					}

					throw new Error(`Gemini returned unexpected format: ${JSON.stringify(data)}`);

				} catch (e) {
					if (_isTransient(0, e.message) && attempt < 2) {
						console.warn(`[AI Client] ${model} attempt ${attempt}/2 threw transient error. Retrying in 3s...`);
						await new Promise(r => setTimeout(r, 3000));
						lastError = e;
						continue;
					}
					// Permanent or final attempt — try next model
					console.warn(`[AI Client] ${model} failed: ${e.message}. Trying next model...`);
					lastError = e;
					break;
				}
			}
		}

		console.error('[AI Client] All models exhausted.', lastError.message);
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

}

module.exports = new AIClient();
