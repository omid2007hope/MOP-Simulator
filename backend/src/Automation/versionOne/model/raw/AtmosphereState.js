const mongoose = require('mongoose');

const atmosphereStateSchema = new mongoose.Schema(
	{
		temperature_K: { type: Number, default: 288.15 },
		pressure_Pa: { type: Number, default: 101325.0 },
		density_kgm3: { type: Number, default: 1.225 },
		speed_of_sound_ms: { type: Number, default: 340.3 },
	},
	{ timestamps: true }
);

module.exports = mongoose.model('AtmosphereState', atmosphereStateSchema);
