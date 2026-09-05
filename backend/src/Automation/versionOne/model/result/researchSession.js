// © 2026 Omid Teimory. All rights reserved.

const mongoose = require('mongoose');

const researchSessionSchema = new mongoose.Schema(
	{
		session_id: { type: String, required: true, unique: true },
		title: { type: String, required: true, trim: true },
		description: { type: String, default: '' },
		cycle_count: { type: Number, required: true },
		status: { type: String, enum: ['in_progress', 'completed', 'failed'], default: 'in_progress' },
		results: [{ type: mongoose.Schema.Types.ObjectId, ref: 'SimulationResultSchema' }]
	},
	{ timestamps: true }
);

module.exports = mongoose.model('ResearchSession', researchSessionSchema);
