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
