// © 2026 Omid Teimory. All rights reserved.

const BaseService = require('./baseservice/index');
const aiClient = require('../../AI/aiClient');
const ResultModel = require('../model/result');
const ArticleModel = require('../model/article');

class ArticleWriter extends BaseService {
	constructor() {
		super(ArticleModel);
	}

	/**
	 * Pulls all simulation results from DB, computes statistics,
	 * calls the AI writer, and saves the generated article.
	 * @param {Object} requestData - { title, limit }
	 */
	async generateResearchArticle(requestData) {
		const { title, limit = 500 } = requestData;

		console.log(`[ArticleWriter] Fetching simulation results for: "${title}"`);

		const results = await ResultModel.find({ research_title: title })
			.sort({ _id: -1 })
			.limit(limit)
			.lean();

		if (results.length === 0) {
			throw new Error('No simulation data found in database. Run POST /research first.');
		}

		console.log(`[ArticleWriter] Analyzing ${results.length} simulation results...`);

		const stats = this._computeStats(results);

		console.log(`[ArticleWriter] Generating research article...`);
		const article = await aiClient.generateArticle(title, stats, results.slice(0, 5));

		const wordCount = article.content.split(/\s+/).length;

		const saved = await this.simplePost({
			title,
			abstract: article.abstract,
			content: article.content,
			word_count: wordCount,
			scenarios_analyzed: results.length,
			avg_penetration_depth: parseFloat(stats.avgPenetrationDepth),
			max_penetration_depth: parseFloat(stats.maxPenetrationDepth),
			dominant_regime: stats.dominantRegime,
			key_findings: article.key_findings,
		});

		console.log(`[ArticleWriter] Article saved (${wordCount} words, ID: ${saved._id})`);

		return {
			article_id: saved._id,
			title,
			word_count: wordCount,
			scenarios_analyzed: results.length,
			stats,
			key_findings: article.key_findings,
			content: article.content,
		};
	}

	/**
	 * Computes aggregate statistics over all simulation result documents.
	 */
	_computeStats(results) {
		const avg = (arr) => arr.reduce((a, b) => a + b, 0) / arr.length;
		const stdDev = (arr) => {
			const mean = avg(arr);
			return Math.sqrt(arr.reduce((s, v) => s + Math.pow(v - mean, 2), 0) / arr.length);
		};

		const depths = results.map((r) => r.cumulative_breach_depth || 0);
		const velocities = results.map((r) => r.velocity || 0);
		const energies = results.map((r) => r.energy || 0);
		const machs = results.map((r) => r.mach || 0);
		const shocks = results.map((r) => r.shock_pressure_gpa_peak || 0);

		// Regime frequency distribution
		const regimeCounts = {};
		results.forEach((r) => {
			const regime = r.regime || 'Unknown';
			regimeCounts[regime] = (regimeCounts[regime] || 0) + 1;
		});
		const dominantRegime =
			Object.entries(regimeCounts).sort((a, b) => b[1] - a[1])[0]?.[0] || 'Unknown';

		return {
			totalScenarios: results.length,
			avgPenetrationDepth: avg(depths).toFixed(2),
			maxPenetrationDepth: Math.max(...depths).toFixed(2),
			minPenetrationDepth: Math.min(...depths).toFixed(2),
			stdDevPenetration: stdDev(depths).toFixed(2),
			avgVelocity: avg(velocities).toFixed(1),
			avgMach: avg(machs).toFixed(2),
			avgEnergyGJ: avg(energies).toFixed(3),
			avgShockPressureGPa: avg(shocks).toFixed(3),
			casingFailureRate: ((results.filter((r) => !r.pressurvives).length / results.length) * 100).toFixed(1),
			erosionRate: ((results.filter((r) => r.erosion_occurred).length / results.length) * 100).toFixed(1),
			regimeDistribution: regimeCounts,
			dominantRegime,
		};
	}
}

module.exports = new ArticleWriter();
