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

const penetrationFrameSchema = new mongoose.Schema(
	{
		t: { type: Number, required: true }, // time
		y: { type: Number, required: true }, // depth
		v: { type: Number, required: true }, // velocity
		m: { type: Number, required: true }, // mach
		p: { type: Number, default: 0 }, // dynamic_pressure (GPa)
		g: { type: Number, default: 0 }, // g_force
		h: { type: Number, default: 0 }, // heat
		e: { type: Boolean, default: false }, // is_eroding
		dif: { type: Number, default: 1 }, // dynamic_increase_factor
		rl: { type: Number, default: 0 }, // remaining_length
		ob: { type: Number, default: 0 }, // obliquity_deg
		cvx: { type: Number, default: 0 }, // current_vx
		cvy: { type: Number, default: 0 }, // current_vy
		up: { type: Number, default: 0 }, // Up (particle velocity)
		us: { type: Number, default: 0 }, // Us (shock velocity)
		ps: { type: Number, default: 0 }, // P_shock
		tp: { type: Number, default: 0 }, // transmitted_pressure
		se: { type: Number, default: 0 }, // shock_energy
		af: { type: Number, default: 0 }, // asymmetric_force
		bm: { type: Number, default: 0 }, // bending_moment
		mbs: { type: Number, default: 0 }, // max_bending_stress
		sr: { type: Number, default: 0 }, // strain_rate
		es: { type: Number, default: 0 }, // effective_strength
		tf: { type: Number, default: 0 }, // tunnel_force
		iev: { type: Number, default: 0 }, // interface_erosion_velocity
		hr: { type: Number, default: 0 }, // heat_rate
		eh: { type: Number, default: 0 }, // excess_heat
		ml: { type: Number, default: 0 }, // mass_loss
		eld: { type: Number, default: 0 }, // effective_linear_density

		// C++ Schema sync fields
		sb: { type: Boolean, default: false }, // is_sonic_boom
		pr: { type: Number, default: 0 }, // pitch_rad
		dc: { type: Number, default: 0 }, // drag_coefficient
		df: { type: Number, default: 0 }, // drag_force
		gp: { type: Number, default: 0 }, // guidance_pull
	},
	{ _id: false },
);

module.exports = penetrationFrameSchema;
