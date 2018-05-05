/*
 * HttpResponse.h
 *
 *  Created on: Apr 20, 2018
 *      Author: pxxian
 */

#ifndef HTTPRESPONSE_H_
#define HTTPRESPONSE_H_

#include <string>
#include <map>

class HttpResponse {
public:
	HttpResponse();
	virtual ~HttpResponse();

	bool parseResponse(const std::string &strResponse);
	std::string getHeaderValueByName(std::string strName);
	bool isStatusSuccess();

	int m_iStatusCode;
	std::string m_strStatusMessage;
	std::map<std::string, std::string> m_mapHeaders;
};

#endif /* HTTPRESPONSE_H_ */
