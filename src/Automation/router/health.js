const express = require('express');
const router = express.Router();

const { health } = require('../controller/health');

router.use('/health', health);

module.exports = router;
