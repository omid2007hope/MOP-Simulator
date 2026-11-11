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

class Status {
	continue = 100;
	switchingProtocols = 101;
	processing = 102;
	earlyHints = 103;

	success = 200;
	created = 201;
	accepted = 202;
	nonAuthoritativeInformation = 203;
	noContent = 204;
	resetContent = 205;
	partialContent = 206;
	multiStatus = 207;
	alreadyReported = 208;
	imUsed = 226;

	multipleChoices = 300;
	movedPermanently = 301;
	found = 302;
	seeOther = 303;
	notModified = 304;
	useProxy = 305;
	temporaryRedirect = 307;
	permanentRedirect = 308;

	badRequest = 400;
	unauthorized = 401;
	paymentRequired = 402;
	forbidden = 403;
	notFound = 404;
	methodNotAllowed = 405;
	notAcceptable = 406;
	proxyAuthenticationRequired = 407;
	requestTimeout = 408;
	conflict = 409;
	gone = 410;
	lengthRequired = 411;
	preconditionFailed = 412;
	payloadTooLarge = 413;
	uriTooLong = 414;
	unsupportedMediaType = 415;
	rangeNotSatisfiable = 416;
	expectationFailed = 417;
	imATeapot = 418;
	misdirectedRequest = 421;
	unprocessableContent = 422;
	locked = 423;
	failedDependency = 424;
	tooEarly = 425;
	upgradeRequired = 426;
	preconditionRequired = 428;
	tooManyRequests = 429;
	requestHeaderFieldsTooLarge = 431;
	unavailableForLegalReasons = 451;

	// Alias kept to support existing controller usage of this.error.
	error = 500;
	internalServerError = 500;
	notImplemented = 501;
	badGateway = 502;
	serviceUnavailable = 503;
	gatewayTimeout = 504;
	httpVersionNotSupported = 505;
	variantAlsoNegotiates = 506;
	insufficientStorage = 507;
	loopDetected = 508;
	notExtended = 510;
	networkAuthenticationRequired = 511;
}

module.exports = Status;
