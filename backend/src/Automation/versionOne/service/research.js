const BaseService = require('./baseservice/index');
const aiClient = require('../../AI/versionOne/aiClient');
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
				// 1. Ask AI to generate hypothesis/scenario parameters
				const config = await aiClient.generateScenario(researchData, i + 1, cycleCount);

				// 2. Feed parameters into the C++ Engine and stream the telemetry out
				console.log(`[Research Loop] Spawning C++ Physics Simulator...`);
				
				const metadata = {
					research_title: researchData.title,
					session_id: sessionId
				};

				// Runner now handles DB insertion internally to prevent OOM
				const totalFrames = await simulationRunner.runSimulation(config, metadata);

				// Retrieve the most recently inserted ResultModel for this session to link it
				const latestResult = await ResultModel.findOne({ session_id: sessionId }).sort({ createdAt: -1 });
				if (latestResult) {
					session.results.push(latestResult._id);
					await session.save();
				}

				allCycleStats.push({
					cycle: i + 1,
					frames_saved: totalFrames,
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
