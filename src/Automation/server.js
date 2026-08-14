const express = require('express');
const app = express();

const asyncHandler = require('./util/asyncHandler');
const createHttpError = require('./util/httpError');

const port = process.env.TEST === true && process.env.PORT ? process.env.PORT : 3000;

if (!process.env.TEST || !process.env.PORT) {
	const missingEnv =
		process.env.TEST === undefined
			? 'Test'
			: process.env.PORT === undefined
				? 'Port'
				: 'An unexpected Error accured';

	throw createHttpError(400, `${missingEnv} is required!`);
}

const health = require('./router/health');

app.listen(port, () => {
	console.Console.log(`Server running on http://localhost:${port}`);
});
