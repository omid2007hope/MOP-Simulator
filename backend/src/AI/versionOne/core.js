const path = require('path');
require('dotenv').config({ path: path.resolve(__dirname, '../Automation/.env') });
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
