const { spawn } = require('child_process');
const path = require('path');
const readline = require('readline');

class SimulationRunner {
    /**
     * Executes the C++ simulator with the given AI-generated configuration.
     * @param {Object} config - The JSON configuration from the AI Client.
     * @returns {Promise<Array>} - Resolves with an array of parsed telemetry frames/results.
     */
    async runSimulation(config) {
        return new Promise((resolve, reject) => {
            // Path to the compiled C++ executable
            const simPath = path.resolve(__dirname, '../../../bin/mop_sim.exe'); 
            
            console.log(`[SimulationRunner] Spawning C++ engine: ${simPath}`);
            const simProcess = spawn(simPath, [], {
                cwd: path.dirname(simPath)
            });

            const results = [];
            let jsonBuffer = '';

            // 1. Setup the Readline Interface for safe memory parsing (Phase 3)
            const rl = readline.createInterface({
                input: simProcess.stdout,
                crlfDelay: Infinity
            });

            rl.on('line', (line) => {
                // If the C++ engine outputs JSON telemetry on stdout, we parse it here.
                // Assuming telemetry frames are printed as single-line JSON strings:
                if (line.trim().startsWith('{') && line.trim().endsWith('}')) {
                    try {
                        const frame = JSON.parse(line);
                        results.push(frame);
                    } catch (e) {
                        // Not a JSON line, or malformed
                    }
                } else {
                    // Pass-through standard console logs for debugging
                    // console.log(`[SIM] ${line}`);
                }
            });

            simProcess.stderr.on('data', (data) => {
                console.error(`[SIM ERROR]: ${data.toString()}`);
            });

            simProcess.on('close', (code) => {
                console.log(`[SimulationRunner] Process exited with code ${code}`);
                if (code !== 0) {
                    reject(new Error(`Simulation failed with code ${code}`));
                } else {
                    resolve(results);
                }
            });

            // 2. Feed the AI config into the C++ Engine's stdin (Phase 2)
            // The C++ main.cpp is highly interactive, requiring sequential std::cin inputs.
            const simChoice = config.Simulation.choice || 3;
            
            // Accept TOS
            simProcess.stdin.write('Y\n');
            
            // Select Mode
            simProcess.stdin.write(`${simChoice}\n`);

            if (simChoice === 1) {
                // Mode 1: Interactive Custom Input
                const p = config.Projectile;
                const t = config.Target.layers[0];
                const s = config.Scenario;

                simProcess.stdin.write(`${p.name}\n`);
                simProcess.stdin.write(`${p.length}\n`);
                simProcess.stdin.write(`${p.diameter}\n`);
                simProcess.stdin.write(`${p.curvature_noseReduce}\n`);
                simProcess.stdin.write(`${p.total_mass}\n`);
                simProcess.stdin.write(`${p.explosive_mass}\n`);
                simProcess.stdin.write(`${p.explosive_energy_j_per_kg}\n`);
                simProcess.stdin.write(`${p.casing_density}\n`);
                simProcess.stdin.write(`${p.yield_strength / 1e9}\n`); // expects GPa
                simProcess.stdin.write(`${p.area_moment_inertia}\n`);
                simProcess.stdin.write(`${p.elastic_modulus / 1e9}\n`); // expects GPa
                simProcess.stdin.write(`${p.casing_wall_thickness}\n`);
                simProcess.stdin.write(`${p.hugoniot_c0}\n`);
                simProcess.stdin.write(`${p.hugoniot_s}\n`);
                simProcess.stdin.write(`${p.explosive_critical_energy / 1e15}\n`); // expects 1e15 Pa^2*s
                
                simProcess.stdin.write(`${t.thickness}\n`);
                simProcess.stdin.write(`${t.rebar_volume_fraction}\n`);
                simProcess.stdin.write(`${t.rebar_yield_strength / 1e6}\n`); // expects MPa
                simProcess.stdin.write(`${t.density}\n`);
                simProcess.stdin.write(`${t.compressive_strength / 1e6}\n`); // expects MPa
                simProcess.stdin.write(`${t.hugoniot_c0}\n`);
                simProcess.stdin.write(`${t.hugoniot_s}\n`);

                simProcess.stdin.write(`${s.numScenarios}\n`);
                
                // Assuming numScenarios = 1 for simplicity in this iteration
                simProcess.stdin.write(`${s.altitude_ft}\n`);
                simProcess.stdin.write(`${s.velocity}\n`);
                simProcess.stdin.write(`${s.obliquity_angle}\n`);
                simProcess.stdin.write(`${s.flight_path_angle}\n`);
                simProcess.stdin.write(`${s.angle_of_attack}\n`);

            } else if (simChoice === 3) {
                // Mode 3: Operation Midnight Hammer
                // Requires hammerChoice (1, 2, or 3)
                const hammerChoice = config.Simulation.numBombs === 2 ? 1 : 
                                     config.Simulation.numBombs === 4 ? 2 : 3;
                simProcess.stdin.write(`${hammerChoice}\n`);
            }

            // Close stdin to let the process run to completion
            simProcess.stdin.end();
        });
    }
}

module.exports = new SimulationRunner();
