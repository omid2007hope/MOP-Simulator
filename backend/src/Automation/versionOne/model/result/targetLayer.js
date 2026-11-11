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
// Signature: OmidTeimory-2026

const mongoose = require('mongoose');

const targetLayerSchema = new mongoose.Schema(
	{
		name: { type: String, required: true }, // material_name
		thickness: { type: Number, required: true },
		density: { type: Number, required: true },
		compressive_strength: { type: Number, default: 0 },
		rebar_volume_fraction: { type: Number, default: 0 },
		rebar_yield_strength: { type: Number, default: 0 },
		pulverized_depth: { type: Number, default: 0 },
		hugoniot_c0: { type: Number, default: 0 },
		hugoniot_s: { type: Number, default: 0 },
		specific_heat: { type: Number, default: 0 },
		melting_point: { type: Number, default: 0 },
		heat_of_fusion: { type: Number, default: 0 },
	},
	{ _id: false },
);

module.exports = targetLayerSchema;
