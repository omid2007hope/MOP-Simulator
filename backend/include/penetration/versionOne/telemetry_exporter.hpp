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

