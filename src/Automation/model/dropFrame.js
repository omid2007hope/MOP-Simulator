// © 2026 Omid Teimory. All rights reserved.
// Signature: OmidTeimory-2026

const mongoose = require('mongoose');

const dropFrameSchema = new mongoose.Schema(
	{
		t: { type: Number, required: true }, // time
		y: { type: Number, required: true }, // altitude
		v: { type: Number, required: true }, // velocity
		m: { type: Number, required: true }, // mach
		sb: { type: Boolean, default: false }, // is_sonic_boom
		pr: { type: Number, default: 0 }, // pitch_rad
		cvx: { type: Number, default: 0 }, // current_vx
		cvy: { type: Number, default: 0 }, // current_vy
		dc: { type: Number, default: 0 }, // drag_coefficient
		df: { type: Number, default: 0 }, // drag_force
		gp: { type: Number, default: 0 }, // guidance_pull
	},
	{ _id: false },
);

module.exports = mongoose.model('DropFrameSchema', dropFrameSchema);
