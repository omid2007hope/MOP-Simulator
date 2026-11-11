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
