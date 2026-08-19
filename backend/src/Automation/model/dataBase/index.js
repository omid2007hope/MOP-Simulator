const path = require('path');
const dotenv = require('dotenv');
const mongoose = require('mongoose');

module.exports = async function connectMongoDB() {
	dotenv.config({ path: path.resolve(__dirname, '../../.env') });

	if (!process.env.MongoDB) {
		throw new Error('MongoDB URI missing. Set MongoDB in .env');
	}

	await mongoose.connect(process.env.MongoDB);
	console.log('MongoDB connected');
	return mongoose.connection;
};
