const path = require('path');
require('dotenv').config({ path: path.resolve(__dirname, '../Automation/.env') });
const asyncHandler = require('../Automation/util/asyncHandler');

const Status = require('../Automation/libs/status');

const apiKey = process.env.API_KEY;

const core = new (class Core extends Status {
	coreHealth = asyncHandler(async (req, res) => {
		if (!apiKey) {
			return res.status(this.error).json({ error: 'API key not configured' });
		}
		res.status(this.success).json({
			message: 'Success',
			key: apiKey ? 'Key loaded' : 'No key',
		});
	});
})();

module.exports = { coreHealth: core.coreHealth };
