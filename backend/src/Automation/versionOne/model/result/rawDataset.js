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
