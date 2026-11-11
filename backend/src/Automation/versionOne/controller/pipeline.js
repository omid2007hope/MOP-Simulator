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
const researchService = require('../service/research');

const PipelineController = new (class PipelineController extends Status {
	/**
	 * Full end-to-end pipeline in one request:
	 *   1. AI generates configs → persisted to DB as raw datasets
	 *   2. C++ engine runs simulation for each config → telemetry streamed to DB
	 *   3. AI analyst synthesizes telemetry
	 *
	 * Body: { title, description?, count? }
	 */
	run = asyncHandler(async (req, res) => {
		const _req = req.body;

		// ─── Step 1 & 2: Research loop (AI config generation + C++ simulation) ───
		console.log('[Pipeline] Step 1/2 — Starting research loop...');
		const researchResult = await researchService.receiveRandomInputs(_req);
		const { session_id } = researchResult;
		console.log(`[Pipeline] Step 1/2 — Done. Session: ${session_id}`);

		res.status(this.created).json({
			session_id,
			research: researchResult,
		});
	});
})();

module.exports = {
	run: PipelineController.run,
};
