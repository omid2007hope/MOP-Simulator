const express = require('express');
const app = express();

const connectMongoDB = require('./model/dataBase/index');

const asyncHandler = require('./util/asyncHandler');
const createHttpError = require('./util/httpError');

if (!process.env.TEST) {
	throw createHttpError(400, 'TEST environment variable is required!');
}

const isTest = process.env.TEST === 'true';

if (!isTest && !process.env.PORT) {
	throw createHttpError(400, 'PORT environment variable is required when not in test mode!');
}

const port = process.env.PORT || (isTest ? 3000 : undefined);
const healthRouter = require('./router/health');
app.use(healthRouter);

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
