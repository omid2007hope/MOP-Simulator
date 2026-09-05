const mongoose = require('mongoose');

const physicsConstantsSchema = new mongoose.Schema(
	{
		gravity: { type: Number, default: 9.81 },
		PI: { type: Number, default: Math.PI },
		frictionFactor: { type: Number, default: 0.1 },
		universalGasConstant: { type: Number, default: 8.31432 },
		molarMassAir: { type: Number, default: 0.0289644 },
		adiabaticIndexAir: { type: Number, default: 1.4 },
		earthRadius: { type: Number, default: 6356766.0 },
		SpeedOfSound: { type: Number, default: 343.0 },
	},
	{ timestamps: true }
);

module.exports = mongoose.model('PhysicsConstants', physicsConstantsSchema);