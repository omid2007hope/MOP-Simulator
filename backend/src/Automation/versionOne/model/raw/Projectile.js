const mongoose = require('mongoose');

const projectileSchema = new mongoose.Schema(
	{
		name: { type: String, required: true, trim: true },
		length: { type: Number, default: 0.0 },
		diameter: { type: Number, default: 0.0 },
		curvature_noseReduce: { type: Number, default: 0.0 },
		total_mass: { type: Number, default: 0.0 },
		explosive_mass: { type: Number, default: 0.0 },
		casing_density: { type: Number, default: 0.0 },
		yield_strength: { type: Number, default: 0.0 },
		area_moment_inertia: { type: Number, default: 0.0 },
		elastic_modulus: { type: Number, default: 0.0 },
		casing_wall_thickness: { type: Number, default: 0.0 },
		hugoniot_c0: { type: Number, default: 0.0 },
		hugoniot_s: { type: Number, default: 0.0 },
		explosive_critical_energy: { type: Number, default: 0.0 },
		explosive_energy_j_per_kg: { type: Number, default: 0.0 },
		specific_heat: { type: Number, default: 460.0 },
		melting_point: { type: Number, default: 1800.0 },
		heat_of_fusion: { type: Number, default: 272000.0 },
	},
	{ timestamps: true }
);

module.exports = mongoose.model('Projectile', projectileSchema);
