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
const { coreHealth } = require('../../AI/core');

app.use(healthRouter);
app.use(researchRouter);
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
