// © 2026 Omid Teimory. All rights reserved.

const mongoose = require('mongoose');

const articleSchema = new mongoose.Schema(
	{
		title: { type: String, required: true, trim: true },
		abstract: { type: String, default: '' },
		content: { type: String, required: true },
		word_count: { type: Number, default: 0 },
		scenarios_analyzed: { type: Number, default: 0 },
		avg_penetration_depth: { type: Number, default: 0 },
		max_penetration_depth: { type: Number, default: 0 },
		dominant_regime: { type: String, default: '' },
		key_findings: [{ type: String }],
	},
	{ timestamps: true },
);

module.exports = mongoose.model('Article', articleSchema);
