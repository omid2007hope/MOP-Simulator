// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#ifndef TELEMETRY_EXPORTER_HPP
#define TELEMETRY_EXPORTER_HPP

// packages
#include <string>
#include <vector>

// files
#include "simulation.hpp"


// ! Namespace for formatting, printing, and exporting simulation telemetry and 3D visualizers
namespace TelemetryExporter {

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

