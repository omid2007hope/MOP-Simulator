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
