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

require('dotenv').config();
const aiClient = require('./src/AI/versionOne/aiClient.js');

async function test() {
    try {
        const result = await aiClient.generateScenario({
            title: "Test",
            description: "Test description"
        }, 1, 1);
        console.log("SUCCESS:");
        console.log(JSON.stringify(result, null, 2));
    } catch (e) {
        console.error("FAILED:", e.message);
    }
}

test();
