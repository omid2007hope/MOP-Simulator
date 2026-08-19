// © 2026 Omid Teimory. All rights reserved.

const asyncHandler = require('../util/asyncHandler');
const Status = require('../libs/status');
const articleWriterService = require('../service/articleWriter');

const ArticleController = new (class ArticleController extends Status {
	generateArticle = asyncHandler(async (req, res) => {
		const _req = req.body;
		const result = await articleWriterService.generateResearchArticle(_req);
		res.status(this.created).json({ data: result });
	});
})();

module.exports = {
	generateArticle: ArticleController.generateArticle,
};
