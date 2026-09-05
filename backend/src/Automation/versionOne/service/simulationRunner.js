const { spawn } = require('child_process');
const path = require('path');
const readline = require('readline');
const fs = require('fs/promises');
const os = require('os');
const crypto = require('crypto');
const ResultModel = require('../model/result/result');

class SimulationRunner {
    /**
     * Executes the C++ simulator with the given AI-generated configuration.
     * Integrates hang protection, OOM streaming to MongoDB, and file-based JSON config.
     * @param {Object} config - The JSON configuration from the AI Client.
     * @param {Object} metadata - { research_title, session_id }
     * @returns {Promise<number>} - Resolves with the total number of frames saved.
     */
    async runSimulation(config, metadata) {
        // Create temporary config file
        const tmpId = crypto.randomBytes(8).toString('hex');
        const tmpConfigPath = path.join(os.tmpdir(), `mop_sim_config_${tmpId}.json`);
        await fs.writeFile(tmpConfigPath, JSON.stringify(config));

        return new Promise((resolve, reject) => {
            const simPath = path.resolve(__dirname, '../../../bin/mop_sim.exe');
            const projectRoot = path.resolve(__dirname, '../../..');

            console.log(`[SimulationRunner] Spawning C++ engine: ${simPath} --json-input`);
            
            // Spawn with --json-input flag
            const simProcess = spawn(simPath, ['--json-input', tmpConfigPath], {
                cwd: projectRoot
            });

            // Hang Protection: 30 second timeout
            const timeout = setTimeout(() => {
                console.error(`[SimulationRunner] Process timed out after 30s! Killing...`);
                simProcess.kill('SIGKILL');
                reject(new Error('Simulation timed out (Infinite loop protection)'));
            }, 30000);

            const rl = readline.createInterface({
                input: simProcess.stdout,
                crlfDelay: Infinity
            });

            simProcess.on('error', (err) => {
                clearTimeout(timeout);
                console.error(`[SimulationRunner] Failed to spawn process: ${err.message}`);
                reject(err);
            });

            simProcess.stderr.on('data', (data) => {
                console.error(`[SIM ERROR]: ${data.toString()}`);
            });

            let totalFrames = 0;
            let chunk = [];
            const CHUNK_SIZE = 1000;
            let streamError = null;

            // Handle lines asynchronously for backpressure support (Prevents Node.js OOM)
            (async () => {
                try {
                    for await (const line of rl) {
                        if (line.trim().startsWith('{') && line.trim().endsWith('}')) {
                            try {
                                const frame = JSON.parse(line);
                                // Inject session metadata to prevent cross-contamination
                                frame.research_title = metadata.research_title || 'Unknown';
                                frame.session_id = metadata.session_id || 'Unknown';
                                
                                chunk.push(frame);
                                totalFrames++;

                                if (chunk.length >= CHUNK_SIZE) {
                                    await ResultModel.insertMany(chunk);
                                    chunk = [];
                                }
                            } catch (e) {
                                console.warn(`[SimulationRunner] Malformed JSON line skipped`);
                            }
                        }
                    }

                    // Insert any remaining frames after stream ends
                    if (chunk.length > 0) {
                        await ResultModel.insertMany(chunk);
                    }
                } catch (err) {
                    streamError = err;
                    clearTimeout(timeout);
                    simProcess.kill();
                }
            })();

            simProcess.on('close', async (code) => {
                clearTimeout(timeout);
                // Clean up temp file
                await fs.unlink(tmpConfigPath).catch(() => {});
                
                if (streamError) {
                    return reject(streamError);
                }

                console.log(`[SimulationRunner] Process exited with code ${code}. Total frames saved: ${totalFrames}`);
                if (code !== 0 && code !== null) { // code is null if killed
                    reject(new Error(`Simulation failed with code ${code}`));
                } else {
                    resolve(totalFrames);
                }
            });
        });
    }
}

module.exports = new SimulationRunner();
