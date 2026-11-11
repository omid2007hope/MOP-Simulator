/*
 * Copyright (c) 2026 Omid Teimory. All Rights Reserved.
 *
 * NOTICE: This file contains proprietary source code licensed exclusively under the
 * terms of the LICENSE file in the root directory of this repository.
 * You may view, download, and compile this code solely to run the unmodified product.
 * Copying, pasting, modifying, redistributing, creating derivative works, or using
 * this code in any other project or AI training system is strictly prohibited.
 * Removal or alteration of this notice is a violation of 17 U.S.C. § 1202 and EU law.
 */

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
