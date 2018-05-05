/*
 * HttpResponse.cpp
 *
 *  Created on: Apr 20, 2018
 *      Author: pxxian
 */

#include <ctype.h>
#include <stdlib.h>
#include <algorithm>
#include "Utils.h"
#include "HttpResponse.h"

HttpResponse::HttpResponse() {

}

HttpResponse::~HttpResponse() {

}

bool HttpResponse::parseResponse(const std::string &strResponse)
{
	size_t nLast = strResponse.find("\r\n\r\n");
	if (nLast >= strResponse.length())
	{
		return false;
	}

	// Get status code and status message
	size_t nPos = 0;
	size_t nFind = strResponse.find("\r\n", nPos);
	if (nFind > nLast)
	{
		return false;
	}

	// response status line
	std::string strStatusLine = strResponse.substr(nPos, nFind - nPos);

	// trim string leading and trailing white spaces
	trim(strStatusLine);

	// get status code and status message
	size_t nBegin = strStatusLine.find(' ');
	if (nBegin > strStatusLine.length())
	{
		return false;
	}

	size_t nEnd = strStatusLine.find_last_of(' ');
	if (nEnd > strStatusLine.length())
	{
		return false;
	}

	std::string strStatusCode = strStatusLine.substr(nBegin + 1, nEnd - nBegin -1);
	m_iStatusCode = atoi(strStatusCode.c_str());
	m_strStatusMessage = strStatusLine.substr(nEnd + 1);

	nPos = nFind + 2;

	// Get Headlers
	while (nPos < nLast)
	{
		nFind = strResponse.find("\r\n", nPos);
		if (nFind > nLast)
		{
			break;
		}

		std::string strItem = strResponse.substr(nPos, nFind - nPos);
		trim(strItem);

		nEnd = strItem.find(':');
		if (nEnd > nFind)
		{
			return false;
		}

		std::string strField = strItem.substr(0, nEnd);
		trim(strField);
		std::transform(strField.begin(), strField.end(), strField.begin(), (int (*)(int))std::tolower);

		std::string strValue = strItem.substr(nEnd + 1);
		trim(strValue);

		m_mapHeaders[strField] = strValue;

		nPos = nFind + 2;
	}

	return true;
}

std::string HttpResponse::getHeaderValueByName(std::string strName)
{
	// to lower
	std::transform(strName.begin(), strName.end(), strName.begin(), (int (*)(int))std::tolower);

	std::string value;
	std::map<std::string, std::string>::const_iterator it = m_mapHeaders.find(strName);
	if (it != m_mapHeaders.end())
	{
		value = it->second;
	}

	return value;
}

bool HttpResponse::isStatusSuccess()
{
	return (m_iStatusCode >= 200 && m_iStatusCode < 300);
}
