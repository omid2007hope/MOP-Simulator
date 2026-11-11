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

const path = require('path');
require('dotenv').config({ path: path.resolve(__dirname, '.env') });

const express = require('express');
const app = express();

const connectMongoDB = require('./model/dataBase/index');

const asyncHandler = require('./util/asyncHandler');
const { createHttpError } = require('./util/httpError');

if (!process.env.TEST) {
	throw createHttpError(400, 'TEST environment variable is required!');
}

const isTest = process.env.TEST === 'true';

if (!isTest && !process.env.PORT) {
	throw createHttpError(400, 'PORT environment variable is required when not in test mode!');
}

const port = process.env.PORT || (isTest ? 3000 : undefined);

// Middleware
app.use(express.json());

// Routers
const healthRouter = require('./router/health');
const researchRouter = require('./router/research');
const simulationDataRouter = require('./router/simulationData');
const pipelineRouter = require('./router/pipeline');
const { coreHealth } = require('../../AI/versionOne/core');

// Serve static files from the root of the backend directory (e.g., for 3d_visualizer.html)
app.use(express.static(path.join(__dirname, '../../..')));

app.use(healthRouter);
app.use(researchRouter);
app.use(simulationDataRouter);
app.use(pipelineRouter);
app.get('/api/health', coreHealth);

connectMongoDB()
	.then(() => {
		app.listen(port, () => {
			console.log(`Server running on http://localhost:${port}`);
		});
	})
	.catch((error) => {
		console.error('Failed to connect to database:', error);
		process.exit(1);
	});

// Global error handler — catches anything forwarded via next(err)
app.use((err, req, res, _next) => {
	const status = err.status || 500;
	res.status(status).json({
		error: err.message || 'Internal Server Error',
		...(err.details !== undefined && { details: err.details }),
	});
});
