const express = require('express');
const app = express();

const asyncHandler = require('./util/asyncHandler');
const createHttpError = require('./util/httpError');

if (!process.env.TEST || !process.env.PORT) {
	const missingEnv = !process.env.TEST
		? 'TEST'
		: !process.env.PORT
			? 'PORT'
			: 'An unexpected error occurred';

	throw createHttpError(400, `${missingEnv} environment variable is required!`);
}

const port = process.env.PORT || 3000;

const healthRouter = require('./router/health');
app.use(healthRouter);

app.listen(port, () => {
	console.log(`Server running on http://localhost:${port}`);
});
