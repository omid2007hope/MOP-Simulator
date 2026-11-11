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

#ifndef TELEMETRY_EXPORTER_HPP
#define TELEMETRY_EXPORTER_HPP

// packages
#include <string>
#include <vector>

// files
#include "simulation.hpp"


// ! Namespace for formatting, printing, and exporting simulation telemetry and 3D visualizers
namespace TelemetryExporter {

	// ! Single source of truth: serializes one SimulationResult to a self-contained JSON string.
	// ! Used by both printReport() (stdout → Node.js) and generateHtml3DVisualizer() (HTML embed).
	std::string serializeResultToJson(const SimulationResult& r,
					  const Projectile& proj,
					  const Target& target);

	void printAscii3DVisualizer(const SimulationResult& r,
				    const Projectile& proj,
				    const Target& target);

	void printReport(const std::vector<SimulationResult>& results,
			 const Projectile& proj,
			 const Target& target);

	void generateHtml3DVisualizer(const std::vector<SimulationResult>& results,
				      const Projectile& proj,
				      const Target& target,
				      const std::string& basePath);

} // namespace TelemetryExporter

#endif // TELEMETRY_EXPORTER_HPP

