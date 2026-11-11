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

const path = require('path');
require('dotenv').config({ path: path.resolve(__dirname, '../../Automation/versionOne/.env') });
const asyncHandler = require('../../Automation/versionOne/util/asyncHandler');

const Status = require('../../Automation/versionOne/libs/status');

const GEMINI_API_KEY = process.env.GEMINI_API_KEY;

const core = new (class Core extends Status {
	coreHealth = asyncHandler(async (req, res) => {
		if (!GEMINI_API_KEY) {
			return res
				.status(this.error)
				.json({ error: 'GEMINI_API_KEY not configured' });
		}
		res.status(this.success).json({
			message: 'Success',
			key: GEMINI_API_KEY ? 'Key loaded' : 'No key',
		});
	});
})();

module.exports = { coreHealth: core.coreHealth };
