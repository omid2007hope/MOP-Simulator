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
     * @param {Object} metadata - { research_title, session_id, rawDataset_id }
     * @returns {Promise<number>} - Resolves with the total number of frames saved.
     */
    async runSimulation(config, metadata) {
        // Create temporary config file
        const tmpId = crypto.randomBytes(8).toString('hex');
        const tmpConfigPath = path.join(os.tmpdir(), `mop_sim_config_${tmpId}.json`);
        await fs.writeFile(tmpConfigPath, JSON.stringify(config));

        return new Promise((resolve, reject) => {
            const simPath = path.resolve(__dirname, '../../../../bin/mop_sim.exe');
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

            /**
             * Maps a C++ drop_frame (full field names) → Mongoose DropFrameSchema (short keys).
             * C++ outputs: time, altitude, velocity, mach, dynamic_pressure, is_sonic_boom, heat,
             *              g_force, pitch_rad, is_eroding, dif, remaining_length, obliquity_deg,
             *              current_vx, current_vy, drag_coefficient, drag_force, guidance_pull
             */
            const mapDropFrame = (f) => ({
                t:   f.time,
                y:   f.altitude,
                v:   f.velocity,
                m:   f.mach,
                p:   f.dynamic_pressure   ?? 0,
                sb:  f.is_sonic_boom      ?? false,
                h:   f.heat               ?? 0,
                g:   f.g_force            ?? 0,
                pr:  f.pitch_rad          ?? 0,
                e:   f.is_eroding         ?? false,
                dif: f.dif               ?? 1,
                rl:  f.remaining_length   ?? 0,
                ob:  f.obliquity_deg      ?? 0,
                cvx: f.current_vx         ?? 0,
                cvy: f.current_vy         ?? 0,
                dc:  f.drag_coefficient   ?? 0,
                df:  f.drag_force         ?? 0,
                gp:  f.guidance_pull      ?? 0,
                up:  f.Up                 ?? 0,
                us:  f.Us                 ?? 0,
                ps:  f.P_shock            ?? 0,
                tp:  f.transmitted_pressure      ?? 0,
                se:  f.shock_energy       ?? 0,
                af:  f.asymmetric_force   ?? 0,
                bm:  f.bending_moment     ?? 0,
                mbs: f.max_bending_stress ?? 0,
                sr:  f.strain_rate        ?? 0,
                es:  f.effective_strength ?? 0,
                tf:  f.tunnel_force       ?? 0,
                iev: f.interface_erosion_velocity ?? 0,
                hr:  f.heat_rate          ?? 0,
                eh:  f.excess_heat        ?? 0,
                ml:  f.mass_loss          ?? 0,
                eld: f.effective_linear_density   ?? 0,
            });

            /**
             * Maps a C++ pen_frame (full field names) → Mongoose PenetrationFrameSchema (short keys).
             * C++ outputs: time, depth, velocity, mach, dynamic_pressure, g_force, heat,
             *              is_eroding, dif, remaining_length, obliquity_deg, current_vx, current_vy,
             *              Up, Us, P_shock, transmitted_pressure, shock_energy, asymmetric_force,
             *              bending_moment, max_bending_stress, strain_rate, effective_strength,
             *              tunnel_force, interface_erosion_velocity, heat_rate, excess_heat,
             *              mass_loss, effective_linear_density
             */
            const mapPenFrame = (f) => ({
                t:   f.time,
                y:   f.depth,
                v:   f.velocity,
                m:   f.mach,
                p:   f.dynamic_pressure   ?? 0,
                g:   f.g_force            ?? 0,
                h:   f.heat               ?? 0,
                e:   f.is_eroding         ?? false,
                dif: f.dif               ?? 1,
                rl:  f.remaining_length   ?? 0,
                ob:  f.obliquity_deg      ?? 0,
                cvx: f.current_vx         ?? 0,
                cvy: f.current_vy         ?? 0,
                up:  f.Up                 ?? 0,
                us:  f.Us                 ?? 0,
                ps:  f.P_shock            ?? 0,
                tp:  f.transmitted_pressure      ?? 0,
                se:  f.shock_energy       ?? 0,
                af:  f.asymmetric_force   ?? 0,
                bm:  f.bending_moment     ?? 0,
                mbs: f.max_bending_stress ?? 0,
                sr:  f.strain_rate        ?? 0,
                es:  f.effective_strength ?? 0,
                tf:  f.tunnel_force       ?? 0,
                iev: f.interface_erosion_velocity ?? 0,
                hr:  f.heat_rate          ?? 0,
                eh:  f.excess_heat        ?? 0,
                ml:  f.mass_loss          ?? 0,
                eld: f.effective_linear_density   ?? 0,
                sb:  f.is_sonic_boom      ?? false,
                pr:  f.pitch_rad          ?? 0,
                dc:  f.drag_coefficient   ?? 0,
                df:  f.drag_force         ?? 0,
                gp:  f.guidance_pull      ?? 0,
            });

            // Handle lines asynchronously for backpressure support (Prevents Node.js OOM)
            const processStream = (async () => {
                try {
                    for await (const line of rl) {
                        if (line.trim().startsWith('{') && line.trim().endsWith('}')) {
                            try {
                                const frame = JSON.parse(line);
                                // Inject session metadata to prevent cross-contamination
                                frame.research_title = metadata.research_title || 'Unknown';
                                frame.session_id = metadata.session_id || 'Unknown';
                                frame.rawDataset_id = metadata.rawDataset_id ? String(metadata.rawDataset_id) : '';

                                // Map C++ full field names → Mongoose short keys for sub-frame arrays
                                if (Array.isArray(frame.drop_frames)) {
                                    frame.drop_frames = frame.drop_frames.map(mapDropFrame);
                                }
                                if (Array.isArray(frame.pen_frames)) {
                                    frame.pen_frames = frame.pen_frames.map(mapPenFrame);
                                }
                                if (Array.isArray(frame.penetration_frames)) {
                                    frame.penetration_frames = frame.penetration_frames.map(mapPenFrame);
                                }
                                
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
                
                // Wait for DB insertion to finish before resolving
                try {
                    await processStream;
                } catch (e) {
                    streamError = streamError || e;
                }

                // Clean up temp file
                try {
                    await fs.unlink(tmpConfigPath);
                } catch (err) {
                    // Ignore error if file doesn't exist
                }
                
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
