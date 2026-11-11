// SPDX-License-Identifier: LicenseRef-MopSimulator-Proprietary
/*
 * Copyright (c) 2026 Omid Teimory. All Rights Reserved.
 *
 * PROPRIETARY SOURCE-AVAILABLE SOFTWARE — NOT OPEN SOURCE.
 * Permission to view, clone, or compile this file is granted strictly on the
 * condition precedent of full compliance with the root ./LICENSE file.
 * Copying snippets, modifying, creating derivative works, redistributing,
 * commercial/SaaS use, and AI/ML training (reserved under EU Directive
 * 2019/790 Art. 4(3) and § 42h(6) Austrian UrhG) are strictly prohibited.
 * Removal or alteration of this notice violates 17 U.S.C. § 1202 and § 90c UrhG.
 */

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
