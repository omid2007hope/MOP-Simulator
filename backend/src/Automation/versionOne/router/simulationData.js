// © 2026 Omid Teimory. All rights reserved.

const express = require('express');
const router = express.Router();

const { generateAndSave } = require('../controller/simulationData');

router.post('/simulationData', generateAndSave);

module.exports = router;
