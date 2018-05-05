/*
 * SocketHttpClient.h
 *
 *  Created on: Apr 20, 2018
 *      Author: pxxian
 */

#ifndef SOCKETHTTPCLIENT_H_
#define SOCKETHTTPCLIENT_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include "HttpClient.h"
#include "HttpUrl.h"
#include "HttpResponse.h"

class SocketHttpClient: public HttpClient {
public:
	SocketHttpClient();
	virtual ~SocketHttpClient();

	virtual bool setUrl(const std::string &strUrl);
	virtual bool getHeadResponse(HttpResponse &resp);
	virtual bool downloadBlock(char* pMem, int nRangeStart, int nRangeSize, bool &bFailed);

private:
	/**
	 * sendStringStream
	 * 		send out the stringstream's message
	 * 	param:
	 * 		fd: socket fd
	 * 		outStream: the string stream for send
	 */
	bool sendStringStream(int fd, std::stringstream & outStream);
	/**
	 * recvString
	 * 		recv the http hander string
	 * param:
	 * 		fd: socket fd
	 * 		strResponse: the string for response
	 * return: int
	 * 		the length of recv message.
	 */
	int recvString(int fd, std::string &strResponse);

	HttpUrl m_url;
	std::string m_strUrl;

	struct sockaddr_in m_soServerAddr;
};

#endif /* SOCKETHTTPCLIENT_H_ */
