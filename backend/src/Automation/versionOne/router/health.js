const express = require('express');
const router = express.Router();

const { health } = require('../controller/health');

router.get('/health', health);

module.exports = router;
