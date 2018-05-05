/*
 * HttpClient.h
 *
 *  Created on: Apr 20, 2018
 *      Author: pxxian
 */

#ifndef HTTPCLIENT_H_
#define HTTPCLIENT_H_

#include <string>
#include "HttpResponse.h"

class HttpClient {
public:
	HttpClient(){}
	virtual ~HttpClient(){}

	/**
	 * setUrl
	 * 		set the http url to HttpClient.
	 */
	virtual bool setUrl(const std::string &strUrl) = 0;
	/**
	 * getHeadResponse
	 * 		get the http head response.
	 */
	virtual bool getHeadResponse(HttpResponse &resp) = 0;
	/**
	 * downloadBlock
	 * 		download block from url
	 * 	param:
	 * 		pMen: save the downloaded block
	 * 		nRangeStart: the Range start
	 * 		nRangeSize: the Range size
	 * 		bFailed: the flag that control downloading
	 * 	return: bool
	 * 		true: success to download
	 * 		false: failed.
	 */
	virtual bool downloadBlock(char* pMen, int nRangeStart, int nRangeSize, bool &bFailed) = 0;
};

#endif /* HTTPCLIENT_H_ */
