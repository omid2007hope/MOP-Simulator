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
