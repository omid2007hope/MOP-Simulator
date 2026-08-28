function createHttpError(status, message, details) {
	const error = new Error(message);
	error.status = status;

	if (details !== undefined) {
		error.details = details;
	}

	return error;
}

module.exports = {
	createHttpError,
};
