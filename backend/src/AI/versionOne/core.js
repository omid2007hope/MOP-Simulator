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
