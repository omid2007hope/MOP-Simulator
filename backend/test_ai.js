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
