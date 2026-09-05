const asyncHandler = require('../util/asyncHandler');
const Status = require('../libs/status');

const { receiveRawData } = require('../service/simulationData');

const SimulationData = new (class SimulationDataController extends Status {
	receiveData = asyncHandler(async (req, res) => {
		const context = req.body;
		const result = await receiveRawData(context);

		res.status(this.success).json({ data: result });
	});
})();

module.exports = {
	receiveRandomInputs: Research.receiveRandomInputs,
};
