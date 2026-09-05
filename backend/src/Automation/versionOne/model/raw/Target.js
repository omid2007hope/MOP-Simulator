const mongoose = require('mongoose');

const targetLayerSchema = new mongoose.Schema(
	{
		material_name: { type: String, required: true, trim: true },
		thickness: { type: Number, default: 0.0 },
		density: { type: Number, default: 0.0 },
		compressive_strength: { type: Number, default: 0.0 },
		rebar_volume_fraction: { type: Number, default: 0.0 },
		rebar_yield_strength: { type: Number, default: 0.0 },
		pulverized_depth: { type: Number, default: 0.0 },
		hugoniot_c0: { type: Number, default: 0.0 },
		hugoniot_s: { type: Number, default: 0.0 },
	},
	{ _id: false } // Typically subdocuments might not need an ID unless specified, but leaving default is fine, let's keep it simple without id false for now. Wait, I will just let mongoose default to true.
);

const targetSchema = new mongoose.Schema(
	{
		name: { type: String, required: true, trim: true },
		layers: [targetLayerSchema],
	},
	{ timestamps: true }
);

targetSchema.methods.pulverizeDepth = function (breachDepth) {
	let currentDepthAcc = 0.0;
	for (let layer of this.layers) {
		if (breachDepth > currentDepthAcc) {
			layer.pulverized_depth = Math.max(
				layer.pulverized_depth,
				Math.min(layer.thickness, breachDepth - currentDepthAcc)
			);
		}
		currentDepthAcc += layer.thickness;
	}
};

module.exports = {
	Target: mongoose.model('Target', targetSchema),
	targetLayerSchema
};
