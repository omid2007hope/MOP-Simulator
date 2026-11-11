// SPDX-License-Identifier: LicenseRef-MopSimulator-Proprietary
/*
 * Copyright (c) 2026 Omid Teimory. All Rights Reserved.
 *
 * PROPRIETARY SOURCE-AVAILABLE SOFTWARE — NOT OPEN SOURCE.
 * Permission to view, clone, or compile this file is granted strictly on the
 * condition precedent of full compliance with the root ./LICENSE file.
 * Copying snippets, modifying, creating derivative works, redistributing,
 * commercial/SaaS use, and AI/ML training (reserved under EU Directive
 * 2019/790 Art. 4(3) and § 42h(6) Austrian UrhG) are strictly prohibited.
 * Removal or alteration of this notice violates 17 U.S.C. § 1202 and § 90c UrhG.
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
