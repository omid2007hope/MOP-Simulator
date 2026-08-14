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
	},
	{ _id: false },
);

module.exports = mongoose.model('TargetLayerSchema', targetLayerSchema);
