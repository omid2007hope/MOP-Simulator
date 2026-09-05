// © 2026 Omid Teimory. All rights reserved.

const BaseService = require('./baseservice/index');
const simulationData = require('./simulationData');
const simulationRunner = require('./simulationRunner');
const ResultModel = require('../model/result/result');
const ResearchSessionModel = require('../model/result/researchSession');
const crypto = require('crypto');

class Research extends BaseService {
	constructor() {
		super(ResultModel);
	}

	async receiveRandomInputs(_req) {
		const cycleCount = _req.count || 3;
		const researchData = _req;
		const sessionId = crypto.randomBytes(4).toString('hex');

		console.log(
			`[Research Loop] Initiating ${cycleCount} autonomous cycles for: ${researchData.title} (Session: ${sessionId})`,
		);

		// Create the tracking session in DB
		const session = await ResearchSessionModel.create({
			session_id: sessionId,
			title: researchData.title,
			description: researchData.description || '',
			cycle_count: cycleCount,
			status: 'in_progress',
		});

		let allCycleStats = [];

		for (let i = 0; i < cycleCount; i++) {
			console.log(
				`\n--- [Research Loop] Starting Cycle ${i + 1} of ${cycleCount} ---`,
			);
			try {
				// 1. Generate AI config and persist the raw dataset to DB (arch.md Item 3)
				const { config, rawDataset_id } = await simulationData.generateAndSave(
					researchData,
					i + 1,
					cycleCount,
					sessionId,
				);

				// 2. Feed config into the C++ Engine, streaming telemetry into MongoDB
				console.log(`[Research Loop] Spawning C++ Physics Simulator...`);

				const metadata = {
					research_title: researchData.title,
					session_id: sessionId,
					rawDataset_id, // arch.md Item 4: link results back to their driving parameters
				};

				const totalFrames = await simulationRunner.runSimulation(config, metadata);

				// 3. Link the most recently inserted result frame to the session
				const latestResult = await ResultModel.findOne({ session_id: sessionId }).sort({ createdAt: -1 });
				if (latestResult) {
					session.results.push(latestResult._id);
					session.rawDatasets.push(rawDataset_id); // arch.md Item 4: bidirectional linkage
					await session.save();
				}

				allCycleStats.push({
					cycle: i + 1,
					frames_saved: totalFrames,
					rawDataset_id,
					status: 'success',
				});
			} catch (error) {
				console.error(
					`[Research Loop] Cycle ${i + 1} Failed: ${error.message}`,
				);
				allCycleStats.push({
					cycle: i + 1,
					error: error.message,
					status: 'failed',
				});
			}
		}

		session.status = 'completed';
		await session.save();

		console.log(`\n[Research Loop] All cycles completed for session ${sessionId}.`);
		return {
			message: 'Autonomous cycles finished',
			session_id: sessionId,
			cycles: allCycleStats,
		};
	}
}

module.exports = new Research();
