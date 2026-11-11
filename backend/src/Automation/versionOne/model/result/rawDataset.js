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
