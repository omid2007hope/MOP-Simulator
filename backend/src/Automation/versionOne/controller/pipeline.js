// © 2026 Omid Teimory. All rights reserved.

const asyncHandler = require('../util/asyncHandler');
const Status = require('../libs/status');
const researchService = require('../service/research');
const articleWriterService = require('../service/articleWriter');

const PipelineController = new (class PipelineController extends Status {
	/**
	 * Full end-to-end pipeline in one request:
	 *   1. AI generates configs → persisted to DB as raw datasets
	 *   2. C++ engine runs simulation for each config → telemetry streamed to DB
	 *   3. AI analyst synthesizes telemetry → AI writer produces research article
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

		// ─── Step 3: Article generation from the completed session ───────────────
		console.log('[Pipeline] Step 3 — Generating research article...');
		const article = await articleWriterService.generateResearchArticle({
			session_id,
		});
		console.log('[Pipeline] Step 3 — Article generated.');

		res.status(this.created).json({
			session_id,
			research: researchResult,
			article,
		});
	});
})();

module.exports = {
	run: PipelineController.run,
};
