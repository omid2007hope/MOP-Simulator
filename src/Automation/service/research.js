const BaseService = require('./baseService/index');
const researchPrompt = require('../../AI/Prompt');

module.exports = new (class Research extends BaseService {
	async receiveRandomInputs(_req) {
		const prompt = researchPrompt;
		return prompt;
	}
})();
