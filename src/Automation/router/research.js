const express = require('express');
const router = express.Router();

const researchController = require('../controller/research');

router.post('/research', researchController.receiveRandomInputs);

module.exports = router;
