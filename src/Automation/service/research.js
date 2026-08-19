const BaseService = require('./baseservice/index');
const aiClient = require('../../AI/aiClient');
const simulationRunner = require('./simulationRunner');
const ResultModel = require('../model/result'); // Assuming this exists to pass to BaseService

class Research extends BaseService {
	constructor() {
		super(ResultModel); // Bind the Mongoose model for db saves
	}

	async receiveRandomInputs(_req) {
		const cycleCount = _req.count || 3;
		const researchData = _req;

		console.log(
			`[Research Loop] Initiating ${cycleCount} autonomous cycles for: ${researchData.title}`,
		);

		let allCycleStats = [];

		for (let i = 0; i < cycleCount; i++) {
			console.log(
				`\n--- [Research Loop] Starting Cycle ${i + 1} of ${cycleCount} ---`,
			);
			try {
				// 1. Ask AI to generate hypothesis/scenario parameters
				const config = await aiClient.generateScenario(researchData);

				// 2. Feed parameters into the C++ Engine and stream the telemetry out
				console.log(`[Research Loop] Spawning C++ Physics Simulator...`);
				const telemetryFrames =
					await simulationRunner.runSimulation(config);

				console.log(
					`[Research Loop] Simulation complete. Captured ${telemetryFrames.length} telemetry frames.`,
				);

				// 3. Batch insert telemetry frames to MongoDB (avoiding 1-by-1 bottlenecks)
				if (telemetryFrames.length > 0) {
					console.log(`[Research Loop] Bulk saving to MongoDB...`);
					await this.simplePost(telemetryFrames);
					console.log(
						`[Research Loop] Database insertion successful.`,
					);
				} else {
					console.log(
						`[Research Loop] No JSON frames detected. (Ensure sim.exe prints valid JSON output).`,
					);
				}

				allCycleStats.push({
					cycle: i + 1,
					frames: telemetryFrames.length,
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

				// In a real system, you might want to pass the error back to the AI to try again,
				// but for now, we continue to the next cycle.
			}
		}

		console.log(`\n[Research Loop] All cycles completed.`);
		return {
			message: 'Autonomous cycles finished',
			cycles: allCycleStats,
		};
	}
}

module.exports = new Research();
