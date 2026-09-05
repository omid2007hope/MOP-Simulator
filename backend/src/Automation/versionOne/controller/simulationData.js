// © 2026 Omid Teimory. All rights reserved.

const asyncHandler = require('../util/asyncHandler');
const Status = require('../libs/status');

const simulationDataService = require('../service/simulationData');

const SimulationDataController = new (class SimulationDataController extends Status {
	// This endpoint is a direct entry point if you want to trigger raw data generation
	// from an HTTP request rather than from within the research loop.
	generateAndSave = asyncHandler(async (req, res) => {
		const { researchData, currentCycle, totalCycles, session_id } = req.body;
		const result = await simulationDataService.generateAndSave(researchData, currentCycle, totalCycles, session_id);

		res.status(this.success).json({ data: result });
	});
})();

module.exports = {
	generateAndSave: SimulationDataController.generateAndSave,
};
