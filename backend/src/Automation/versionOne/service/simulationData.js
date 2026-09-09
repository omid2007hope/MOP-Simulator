// © 2026 Omid Teimory. All rights reserved.

const BaseService = require('./baseservice/index');
const aiClient = require('../../../AI/versionOne/aiClient');
const RawDatasetModel = require('../model/result/rawDataset');

class SimulationData extends BaseService {
	constructor() {
		super(RawDatasetModel);
	}

	/**
	 * Generates a simulation config via the AI and persists it to the database.
	 * Satisfies arch.md: "Raw Data Storage — AI-generated raw datasets are saved into the database."
	 * @param {Object} researchData - { title, description, count }
	 * @param {Number} currentCycle - Current cycle index (1-indexed)
	 * @param {Number} totalCycles - Total number of cycles
	 * @param {String} session_id - Parent session ID for data linkage
	 * @returns {{ config: Object, rawDataset_id: ObjectId }} AI config payload and its DB record ID
	 */
	async generateAndSave(researchData, currentCycle, totalCycles, session_id) {
		// 1. Ask the AI to generate the full simulation config
		const config = await aiClient.generateScenario(researchData, currentCycle, totalCycles);

		// 2. Persist the raw config to DB before it reaches the C++ engine
		const saved = await this.simplePost({
			session_id,
			cycle: currentCycle,
			payload: {
				PhysicsConstants: config.PhysicsConstants || {},
				AtmosphereState: config.AtmosphereState || {},
				Aircraft: config.Aircraft || {},
				Projectile: config.Projectile || {},
				Target: config.Target || {},
				Scenarios: config.Scenarios || [],
			},
		});

		console.log(`[SimulationData] Raw dataset saved for cycle ${currentCycle} (ID: ${saved._id})`);

		return { config, rawDataset_id: saved._id };
	}
}

module.exports = new SimulationData();
