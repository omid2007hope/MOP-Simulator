const mongoose = require('mongoose');
const SimulationResult = require('./src/Automation/versionOne/model/result/result');
console.log("Paths in schema:", Object.keys(SimulationResult.schema.paths));
