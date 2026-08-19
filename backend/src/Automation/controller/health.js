const asyncHandler = require('../util/asyncHandler');

const health = asyncHandler(async (req, res) => {
	const result = 'Okay!';
	res.status(200).json({ data: result });
});

module.exports = {
	health,
};
