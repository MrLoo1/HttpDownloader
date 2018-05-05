/*
 * HttpUrl.h
 *
 *  Created on: Apr 20, 2018
 *      Author: pxxian
 */

#ifndef HTTPURL_H_
#define HTTPURL_H_

#include <string>
#include <netdb.h>

class HttpUrl {
public:
	HttpUrl();
	virtual ~HttpUrl();

	/**
	 * parse http url
	 */
	bool parseUrl(std::string strUrl);

	std::string m_strHost;
	std::string m_strDomain;
	uint m_uPort;
	std::string m_strUri;
	struct hostent* m_pHostent;
};

#endif /* HTTPURL_H_ */
