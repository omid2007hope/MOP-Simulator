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
