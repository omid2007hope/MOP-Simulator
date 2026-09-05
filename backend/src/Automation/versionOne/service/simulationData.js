const BaseService = require('./baseservice/index');

// 1. Import all the raw models
const Aircraft = require('../model/raw/Aircraft');
const AtmosphereState = require('../model/raw/AtmosphereState');
const PhysicsConstants = require('../model/raw/PhysicsConstants');
const Projectile = require('../model/raw/Projectile');
const Scenarios = require('../model/raw/Scenarios');
const { Target } = require('../model/raw/Target'); // Destructure because Target.js exports an object

class SimulationData extends BaseService {
	constructor() {
		// 2. Pass your primary model to super() if this service manages one specific collection
		// (For example, if you create a RawDataset model to hold all these together, you'd pass super(RawDatasetModel))
		super();

		// 3. Attach them to the instance so you can access them anywhere in this service via this.rawModels
		this.rawModels = {
			Aircraft,
			AtmosphereState,
			PhysicsConstants,
			Projectile,
			Scenarios,
			Target,
		};
	}
}

module.exports = new SimulationData();
