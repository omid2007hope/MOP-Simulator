class BaseService {
	constructor(model) {
		this.model = model;
	}

	async simplePost(data) {
		try {
			return await this.model.create(data);
		} catch (error) {
			console.error(error);
			throw new Error('Post was unsuccessful');
		}
	}

	async simplePatch(data, id, isNew = true) {
		try {
			return await this.model.findOneAndUpdate(id, data, {
				returnDocument: isNew ? 'After' : 'Before',
			});
		} catch (error) {
			console.error(error);
			throw new Error('Patch was unsuccessful');
		}
	}

	async simpleGet() {
		try {
			return await this.model.find();
		} catch (error) {
			console.error(error);
			throw new Error('Could not get any data');
		}
	}
}

module.exports = BaseService;
