const mongoose = require('mongoose');
const ResultModel = require('./src/Automation/versionOne/model/result/result');

async function test() {
    await mongoose.connect('mongodb://localhost:27017/MopSimDB');
    console.log("Connected to MongoDB.");
    const coll = mongoose.connection.db.collection('simulationresults');
    const options = await coll.options();
    console.log("Collection options (validation):", JSON.stringify(options, null, 2));

    try {
        await ResultModel.create({ name: "Test Validation", bombers_horizontal_velocity: 100, mach: 1, energy: 1 });
        console.log("Insert SUCCESS!");
    } catch (e) {
        console.error("Insert FAILED:", e.message);
    }
    process.exit(0);
}

test();
