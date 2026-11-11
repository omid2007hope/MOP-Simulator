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

const asyncHandler = require('../util/asyncHandler');
const Status = require('../libs/status');

const researchService = require('../service/research');

const Research = new (class Research extends Status {
	receiveRandomInputs = asyncHandler(async (req, res) => {
		const _req = req.body;
		const result = await researchService.receiveRandomInputs(_req);

		res.status(this.success).json({ data: result });
	});
})();

module.exports = {
	receiveRandomInputs: Research.receiveRandomInputs,
};
