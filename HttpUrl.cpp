/*
 * HttpUrl.cpp
 *
 *  Created on: Apr 20, 2018
 *      Author: pxxian
 */

#include <algorithm>
#include <string.h>
#include "Utils.h"
#include "Logger.h"
#include "HttpUrl.h"

HttpUrl::HttpUrl() {
	m_pHostent = NULL;
}

HttpUrl::~HttpUrl() {

}

bool HttpUrl::parseUrl(std::string strUrl)
{
	if (strUrl.empty())
	{
		return false;
	}

	// trim string leading and trailing white spaces.
	trim(strUrl);

	// parse the url and port
	std::string strTempUrl = strUrl;
	std::transform(strTempUrl.begin(), strTempUrl.end(), strTempUrl.begin(), (int (*)(int))std::tolower);

	size_t uFind = strTempUrl.find("http://");
	if (uFind != 0)
	{
		Logger::getSingleton()->log("Error: Invalid URL:%s", strUrl.c_str());
	    return false;
	}

	int nLen = strlen("http://");
	uFind = strTempUrl.find('/', nLen);
	if (uFind > strTempUrl.length() )
	{
		Logger::getSingleton()->log("Error: Invalid URL(2):%s", strUrl.c_str());
	    return false;
	}

	m_strHost = strUrl.substr(nLen, uFind - nLen);
	m_strUri  = strUrl.substr(uFind);

	m_strDomain = m_strHost;
	m_uPort = 80;

	uFind = m_strHost.find(':');
	if (uFind < m_strHost.length())
	{
	    m_strDomain = m_strHost.substr( 0, uFind);
	    m_uPort = atoi(m_strHost.substr(uFind+1).c_str());
	}

	m_pHostent = gethostbyname(m_strDomain.c_str());
	if (m_pHostent == NULL)
	{
		Logger::getSingleton()->log("Error: Failed to resolve the IP address for the URL:%s", strUrl.c_str());
		return false;
	}

	return true;
}
