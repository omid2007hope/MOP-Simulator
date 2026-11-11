/*
 * Copyright (c) 2026 Omid Teimory. All Rights Reserved.
 *
 * NOTICE: This file contains proprietary source code licensed exclusively under the
 * terms of the LICENSE file in the root directory of this repository.
 * You may view, download, and compile this code solely to run the unmodified product.
 * Copying, pasting, modifying, redistributing, creating derivative works, or using
 * this code in any other project or AI training system is strictly prohibited.
 * Removal or alteration of this notice is a violation of 17 U.S.C. § 1202 and EU law.
 */

// © 2026 Omid Teimory. All rights reserved.

const mongoose = require('mongoose');

const researchSessionSchema = new mongoose.Schema(
	{
		session_id: { type: String, required: true, unique: true },
		title: { type: String, required: true, trim: true },
		description: { type: String, default: '' },
		cycle_count: { type: Number, required: true },
		status: { type: String, enum: ['in_progress', 'completed', 'failed'], default: 'in_progress' },
		results: [{ type: mongoose.Schema.Types.ObjectId, ref: 'SimulationResult' }],
		rawDatasets: [{ type: mongoose.Schema.Types.ObjectId, ref: 'RawDataset' }], // Links each cycle's raw config to this session
	},
	{ timestamps: true }
);

module.exports = mongoose.model('ResearchSession', researchSessionSchema);
