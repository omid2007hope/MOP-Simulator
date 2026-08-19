// © 2026 Omid Teimory. All rights reserved.

const express = require('express');
const router = express.Router();

const { generateArticle } = require('../controller/article');

router.post('/article', generateArticle);

module.exports = router;
