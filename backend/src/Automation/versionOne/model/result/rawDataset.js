// © 2026 Omid Teimory. All rights reserved.

const mongoose = require('mongoose');

// Stores the exact AI-generated JSON payload for each research cycle.
// Satisfies arch.md: "Raw Data Storage — AI-generated raw datasets are saved into the database."
const rawDatasetSchema = new mongoose.Schema(
	{
		session_id: { type: String, required: true, index: true },
		cycle: { type: Number, required: true },
		payload: {
			PhysicsConstants: { type: mongoose.Schema.Types.Mixed, default: {} },
			AtmosphereState: { type: mongoose.Schema.Types.Mixed, default: {} },
			Aircraft: { type: mongoose.Schema.Types.Mixed, default: {} },
			Projectile: { type: mongoose.Schema.Types.Mixed, default: {} },
			Target: { type: mongoose.Schema.Types.Mixed, default: {} },
			Scenarios: { type: [mongoose.Schema.Types.Mixed], default: [] },
		},
	},
	{ timestamps: true },
);

module.exports = mongoose.model('RawDataset', rawDatasetSchema);
