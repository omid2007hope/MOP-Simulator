const mongoose = require('mongoose');

const impactScenarioSchema = new mongoose.Schema(
	{
		name: { type: String, required: true, trim: true },
		altitude_ft: { type: Number, default: 0.0 },
		velocity: { type: Number, default: 0.0 },
		flight_path_angle: { type: Number, default: 0.0 },
		obliquity_angle: { type: Number, default: 0.0 },
		angle_of_attack: { type: Number, default: 0.0 },
	},
	{ timestamps: true },
);

module.exports = mongoose.model('ImpactScenario', impactScenarioSchema);
