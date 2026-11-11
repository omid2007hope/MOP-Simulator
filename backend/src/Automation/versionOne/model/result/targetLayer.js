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
