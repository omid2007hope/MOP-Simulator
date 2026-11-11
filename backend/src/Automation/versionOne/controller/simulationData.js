// SPDX-License-Identifier: LicenseRef-MopSimulator-Proprietary
/*
 * Copyright (c) 2026 Omid Teimory. All Rights Reserved.
 *
 * PROPRIETARY SOURCE-AVAILABLE SOFTWARE — NOT OPEN SOURCE.
 * Permission to view, clone, or compile this file is granted strictly on the
 * condition precedent of full compliance with the root ./LICENSE file.
 * Copying snippets, modifying, creating derivative works, redistributing,
 * commercial/SaaS use, and AI/ML training (reserved under EU Directive
 * 2019/790 Art. 4(3) and § 42h(6) Austrian UrhG) are strictly prohibited.
 * Removal or alteration of this notice violates 17 U.S.C. § 1202 and § 90c UrhG.
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
