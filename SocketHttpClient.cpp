/*
 * SocketHttpClient.cpp
 *
 *  Created on: Apr 20, 2018
 *      Author: pxxian
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "SocketHttpClient.h"
#include "Logger.h"

#define SOCK_TIMEOUT 15 	// seconds. socket timeout
#define MAX_ERR_TIMES 5		// times. download's socket error

SocketHttpClient::SocketHttpClient() {

}

SocketHttpClient::~SocketHttpClient() {
}

bool SocketHttpClient::setUrl(const std::string &strUrl)
{
	m_strUrl = strUrl;
	// parse URL
	if(!m_url.parseUrl(m_strUrl))
	{
		return false;
	}

	// make a ServerAddr
	memset(&m_soServerAddr, 0, sizeof(m_soServerAddr));
	m_soServerAddr.sin_family = AF_INET;
	m_soServerAddr.sin_port = htons((short)m_url.m_uPort);
	memcpy((char*)&m_soServerAddr.sin_addr.s_addr, m_url.m_pHostent->h_addr_list[0], m_url.m_pHostent->h_length);

	return true;
}

bool SocketHttpClient::getHeadResponse(HttpResponse &resp)
{
	// create socket
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == fd)
	{
		Logger::getSingleton()->log("Error: socket failed, err msg=%s", strerror(errno));
		return false;
	}
	// connect
	int nErr = connect(fd, (struct sockaddr*)&m_soServerAddr, sizeof(struct sockaddr));
	if (nErr == -1)
	{
		close(fd);
		Logger::getSingleton()->log("Error: failed to connect to Url: %s", m_strUrl.c_str());
		return false;
	}

	// set timeout
	struct timeval tv = {0};
	tv.tv_sec = SOCK_TIMEOUT;
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv)))
	{
		close(fd);
		Logger::getSingleton()->log("Error: failed to setsockopt. error=%s", strerror(errno));
		return false;
	}

	// http head request
	std::stringstream strHttp;
	strHttp << "HEAD " << m_url.m_strUri << " HTTP/1.1\r\n";
	strHttp << "Host:" << m_url.m_strHost << "\r\n";
	strHttp << "Cache-Control:no-cache\r\n";
	strHttp << "Pragma: no-cache\r\n";
	strHttp << "User-Agent:Mozilla/4.0\r\n";
	strHttp << "Connection:Keep-Alive\r\n";
	strHttp << "Accept:*/*\r\n";
	strHttp << "\r\n";

	// send
	if (!sendStringStream(fd, strHttp))
	{
		close(fd);
		Logger::getSingleton()->log("Error: Failed to send the Http Head request to URL: %s", m_strUrl.c_str());
		return false;
	}
	// recv
	std::string strResponse;
	recvString(fd, strResponse);
	shutdown(fd, SHUT_RDWR);
	close(fd);

	// parse the http head response
	if (!resp.parseResponse(strResponse))
	{
		Logger::getSingleton()->log("Error: The Http Head Response contains nothing. Url:%s", m_strUrl.c_str());
		return false;
	}
	// check
	if (!resp.isStatusSuccess())
	{
		Logger::getSingleton()->log("Error: Status Code:%d. Url:%s", resp.m_iStatusCode, m_strUrl.c_str());
		return false;
	}

	return true;
}

bool SocketHttpClient::downloadBlock(char* pMem, int nRangeStart, int nRangeSize, bool &bFailed)
{
	Logger::getSingleton()->log("Info: download block \"%s\" [%08d-%08d] start...",
		 m_strUrl.c_str(), nRangeStart, nRangeStart + nRangeSize - 1);

	int nReceived = 0;
	int nErrTimes = 0;
	while (nReceived < nRangeSize && nErrTimes < MAX_ERR_TIMES && !bFailed)
	{
		int fd = socket( AF_INET, SOCK_STREAM, 0);
		if( fd == -1 )
		{
			nErrTimes++;
			continue;
		}

		int nErr = connect(fd, (struct sockaddr *)&m_soServerAddr, sizeof(struct sockaddr));
		if (nErr == -1)
		{
			close(fd);
			nErrTimes++;
			Logger::getSingleton()->log("Error: failed to connect to URL:%s" , m_strUrl.c_str());
			continue;
		}

		// set timeout
		struct timeval tv = {0};
		tv.tv_sec = SOCK_TIMEOUT;
		if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,  sizeof(tv)))
		{
			close(fd);
			nErrTimes++;
			Logger::getSingleton()->log("Error: setsockopt failed(2). error=%s", strerror(errno));
			continue;
		}

		// http get request
		std::stringstream strHttp;
		strHttp << "GET " << m_url.m_strUri << " HTTP/1.1\r\n";
		strHttp << "Host:" << m_url.m_strHost << "\r\n";
		strHttp << "Cache-Control:no-cache\r\n";
		strHttp << "Pragma: no-cache\r\n";
		strHttp << "User-Agent:Mozilla/4.0\r\n";
		strHttp << "Connection:Keep-Alive\r\n";
		strHttp << "Accept:*/*\r\n";
		strHttp << "Range: bytes=" << nRangeStart+nReceived << "-" << nRangeStart+nRangeSize-1 <<"\r\n";
		strHttp << "\r\n";

		// send
		if (!sendStringStream(fd, strHttp))
		{
			shutdown(fd, SHUT_RDWR);
			close(fd);
			Logger::getSingleton()->log("Error: failed to send the HTTP GET request to URL:%s", m_strUrl.c_str());
			nErrTimes++;
			continue;
		}
		// recv
		char szBuf[1024] = {0};
		nErr = recv(fd, szBuf, 1024, 0);
		if (nErr <= 0)
		{
			shutdown(fd, SHUT_RDWR);
			close(fd);
			Logger::getSingleton()->log("Error: recv failed from URL=%s. return code=%d, error=%s",
					m_strUrl.c_str(), nErr, strerror(errno));
			nErrTimes++;
			continue;
		}
		// Whether the header is received
		int nRemain = 0;
		int nIndex = 0;
		std::string strResponse;
		for(nIndex = 0; nIndex < nErr; nIndex++)
		{
			if( szBuf[nIndex] == '\r' &&
				szBuf[nIndex+1] == '\n' &&
				szBuf[nIndex+2] == '\r' &&
				szBuf[nIndex+3] == '\n' )
			{
				char szTemp[1025] = {0};
				memcpy(szTemp, szBuf, nIndex+4);
				strResponse = szTemp;
				nRemain = nErr -(nIndex+4);
				break;
			}
		}
		// check the response
		if( strResponse.length() == 0 )
		{
			shutdown(fd, SHUT_RDWR);
			close(fd);
			nErrTimes++;
			Logger::getSingleton()->log("Error: the response does not contain a HTTP header, URL:%s", m_strUrl.c_str());
			continue;
		}

		HttpResponse resp;
		if (!resp.parseResponse(strResponse))
		{
			shutdown(fd, SHUT_RDWR);
			close(fd);
			nErrTimes++;
			Logger::getSingleton()->log("Error: the response does not contain a HTTP header(2), URL:%s", m_strUrl.c_str());
			continue;
		}

		if (!resp.isStatusSuccess())
		{
			shutdown(fd, SHUT_RDWR);
			close(fd);
			nErrTimes++;
			Logger::getSingleton()->log("Error: StatusCode:%d. URL:%s", resp.m_iStatusCode, m_strUrl.c_str());
			continue;
		}
		// write down the block message
		if (nRemain > 0)
		{
			memcpy((char*)(pMem+nReceived), &(szBuf[nIndex+4]), nRemain);
			nReceived += nRemain;
		}
		// Receive the remaining part
		while( (nReceived < nRangeSize) && !bFailed )
		{
			nErr = recv(fd, (unsigned char*)(pMem+nReceived), nRangeSize - nReceived, 0);
			if( nErr <= 0 )
			{
				Logger::getSingleton()->log("Error: recv failed(2) from URL=%s. err=%d, msg=%s",
									m_strUrl.c_str(), nErr, strerror(errno));
				nErrTimes++;
				break;
			}
			nReceived += nErr;
		}
		shutdown(fd, SHUT_RDWR);
		close(fd);
	}// while

	// check
	if (nReceived != nRangeSize)
	{
		bFailed = true;
	}

	Logger::getSingleton()->log("Info: download block \"%s\" [%08d-%08d] %s.",
			m_strUrl.c_str(), nRangeStart, nRangeStart + nRangeSize - 1,
			(nReceived == nRangeSize) ?  "success" : "failed");
	return (nReceived == nRangeSize);
}

bool SocketHttpClient::sendStringStream(int fd, std::stringstream & outStream)
{
	int nSize = outStream.str().length();
	char* pBuf = new char[nSize+1];
	memcpy(pBuf, outStream.str().c_str(), nSize);
	pBuf[nSize] = '\0';

//	Logger::getSingleton()->log("Debug: buffer:\n%s\nsize=%d", pBuf, nSize);
	int nSent = 0;
	while (nSent < nSize)
	{
		int nErr = send(fd, (char*) (pBuf + nSent), nSize - nSent, 0);
		if (nErr == -1)
		{
			break;
		}

		nSent += nErr;
	}

	delete[] pBuf;

//	Logger::getSingleton()->log("Debug: sent=%d, size=%d", nSent, nSize);

	return (nSent ==  nSize);
}

int SocketHttpClient::recvString(int fd, std::string &strResponse)
{
	int nRecv = 0;
	std::stringstream getStream;
	while (true)
	{
		char cBuf[1025] = {0};
		int nErr = recv(fd, cBuf, 1024, 0);
		if (nErr <= 0)
			break;
		getStream << cBuf;
		nRecv += nErr;

		if (getStream.str().find("\r\n\r\n") != std::string::npos)
			break;
	}

	strResponse = getStream.str();

	Logger::getSingleton()->log("Debug: buffer:\n%s", strResponse.c_str());

	return nRecv;
}
