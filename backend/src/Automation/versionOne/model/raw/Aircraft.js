const mongoose = require('mongoose');

const aircraftSchema = new mongoose.Schema(
	{
		name: { type: String, required: true, trim: true },
		bomber_totalMass: { type: Number, default: 0.0 },
		bomber_wingArea: { type: Number, default: 0.0 },
		bomber_liftCurveSlope: { type: Number, default: 0.0 },
	},
	{ timestamps: true }
);

module.exports = mongoose.model('Aircraft', aircraftSchema);
