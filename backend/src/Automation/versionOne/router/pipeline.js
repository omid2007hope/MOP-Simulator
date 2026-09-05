// © 2026 Omid Teimory. All rights reserved.

const express = require('express');
const router = express.Router();

const { run } = require('../controller/pipeline');

router.post('/pipeline', run);

module.exports = router;
