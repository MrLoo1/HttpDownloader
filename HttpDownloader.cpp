/*
 * HttpDownloader.cpp
 *
 *  Created on: Apr 20, 2018
 *      Author: pxxian
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include "HttpDownloader.h"
#include "SocketHttpClient.h"
#include "Logger.h"
#include "Utils.h"

#define MAX_THREAD 5
#define LOOP_DELAY 1000 //micro_seconds

typedef struct DownloadTask
{
	HttpDownloader* pThis;
	char* pMemStart;
	int nSize;
	int nRangeStart;
} DownloadTask;

HttpDownloader::HttpDownloader() {

	m_bCompleted = false;
	m_bSuccess = false;
	m_httpClient = new SocketHttpClient();
}

HttpDownloader::~HttpDownloader() {

	if (m_httpClient != NULL)
	{
		delete m_httpClient;
		m_httpClient = NULL;
	}
}

bool HttpDownloader::isCompleted(bool & bResult)
{
	bResult = m_bSuccess;
	return m_bCompleted;
}

bool HttpDownloader::downloadUrlToFile(const std::string &strUrl, const std::string &strLoadFile)
{
	if (strUrl.empty() || strLoadFile.empty())
	{
		return false;
	}

	m_strUrl = strUrl;
	m_strLoadFile = strLoadFile;
	Logger::getSingleton()->log("Info: download file from: %s to %s", m_strUrl.c_str(), m_strLoadFile.c_str());

	// create leader thread
	int nErr = pthread_create(&m_ptLeaderThread, NULL, &leaderThread, this);
	if (nErr != 0)
	{
		Logger::getSingleton()->log("Error: pthread_create leaderThread failed, return=%d, message=%s",
				nErr, strerror(nErr));
		return false;
	}

	return true;
}

void* HttpDownloader::leaderThread(void* param)
{
	HttpDownloader* pThis = static_cast<HttpDownloader*>(param);

	Logger::getSingleton()->log("Info: download file from: %s start ...", pThis->m_strUrl.c_str());

	pThis->m_bCompleted = false;
	// process download task
	pThis->m_bSuccess = pThis->downloadProcess();

	pThis->m_bCompleted = true;

	Logger::getSingleton()->log("Info: download file form : %s, %s",
			pThis->m_strUrl.c_str(), pThis->m_bSuccess? "success":"failed");

	return NULL;
}

bool HttpDownloader::downloadProcess()
{
	// Get Content-Length and Content-MD5
	// 1. set http url
	if (!m_httpClient->setUrl(m_strUrl))
	{
		Logger::getSingleton()->log("Error: Can't parseUrl url:%s", m_strUrl.c_str());
		return false;
	}
	// 2. get http head response
	HttpResponse resp;
	if (!m_httpClient->getHeadResponse(resp))
	{
		Logger::getSingleton()->log("Error: Failed to Get Http Head Response. Url:%s", m_strUrl.c_str());
		return false;
	}
	// 3. get Content-Length string
	std::string strContentLength = resp.getHeaderValueByName("Content-Length");
	if (strContentLength.empty())
	{
		Logger::getSingleton()->log("Error: Invalid Content-Length in Response. Url:%s", resp.m_iStatusCode, m_strUrl.c_str());
		return false;
	}
	// 4. string to int
	int nContentLength = atoi(strContentLength.c_str());
	if (nContentLength <= 0)
	{
		Logger::getSingleton()->log("Error: Can't get Content-Length from: url:%s", m_strUrl.c_str());
		return false;
	}
	// 5. get Content-MD5 string
	std::string strMD5 = resp.getHeaderValueByName("Content-MD5");

	// note: If not support block download, use single thread downloading.
	// But this time it doesn't come true, you should do it on your own.

	// create local file with size=nContentLength
	createFile(m_strLoadFile, nContentLength);

	int fd = open(m_strLoadFile.c_str(), O_RDWR);
	if (fd == -1)
	{
		Logger::getSingleton()->log("Error: Can't create file:%s", m_strLoadFile.c_str());
		return false;
	}

	// Memory map
	char* pMem = (char*) mmap(NULL, nContentLength, PROT_WRITE, MAP_SHARED | MAP_POPULATE | MAP_NONBLOCK, fd, 0);
	close(fd);

	if (pMem == MAP_FAILED)
	{
		Logger::getSingleton()->log("Error: Failed to map the file: %s into memory; size=%d; err msg=%s",
				m_strLoadFile.c_str(), nContentLength, strerror(errno));
		return false;
	}
	mlock(pMem, nContentLength);

	// Create download thread
	pthread_mutex_init(&m_pmDownload, NULL);
	m_bFailed = false;
	int nDownloadLen = 0;
	m_nThreadCnt = 0;

	int nBlockSize = nContentLength / MAX_THREAD;
	if (nContentLength % MAX_THREAD != 0)
		nBlockSize += 1;

	while (nDownloadLen < nContentLength && !m_bFailed)
	{
		// make a DownloadTask
		DownloadTask *pTask = (DownloadTask*) malloc(sizeof(DownloadTask));
		pTask->pMemStart = pMem + nBlockSize;
		pTask->nSize = ((nDownloadLen + nBlockSize) > nContentLength)?
				(nContentLength - nDownloadLen) : nBlockSize;
		pTask->nRangeStart = nDownloadLen;
		pTask->pThis = this;

		nDownloadLen += pTask->nSize;

		// create thread
		pthread_t pDownload;
		int nErr = pthread_create(&pDownload, NULL, &downloadThread, pTask);
		if (nErr != 0)
		{
			Logger::getSingleton()->log("Error: pthread_create download thread failed. Error=%d, Msg=%s",
					nErr, strerror(nErr));
			m_bFailed = true;
		}
		else
		{
			pthread_mutex_lock(&m_pmDownload);
			m_nThreadCnt++;
			pthread_mutex_unlock(&m_pmDownload);
		}

	}

	while (m_nThreadCnt != 0)
	{
		usleep(LOOP_DELAY);
	}

	pthread_mutex_destroy(&m_pmDownload);

	// MD5
	if (!strMD5.empty() && !m_bFailed)
	{
		unsigned char ucDM5[16];
		MD5((unsigned char*) pMem, nContentLength, ucDM5);
		std::string strTemp = base64_encode(ucDM5, 16);

//		Logger::getSingleton()->log("Debug: DM5: \"url_md5=%s\", \"now_md5=%s\"",
//							strMD5.c_str(), strTemp.c_str());

		if (strTemp != strMD5)
		{
			Logger::getSingleton()->log("Error: Failed to check DM5: \"url_md5=%s\", \"now_md5=%s\"",
					strMD5.c_str(), strTemp.c_str());

			m_bFailed = true;
		}
	}

	// sync
	if (-1 == msync(pMem, nContentLength, MS_SYNC))
	{
		Logger::getSingleton()->log("Error: Failed to msync the file: %s from memory; size=%d; err msg=%s",
				m_strLoadFile.c_str(), nContentLength, strerror(errno));
		m_bFailed = true;
	}


	munlock(pMem, nContentLength);
	munmap(pMem, nContentLength);

	return !m_bFailed;
}

void* HttpDownloader::downloadThread(void* param)
{
	DownloadTask* pTask = static_cast<DownloadTask*>(param);

	// download block
	pTask->pThis->m_httpClient->downloadBlock(pTask->pMemStart, pTask->nRangeStart, pTask->nSize, pTask->pThis->m_bFailed);

	pthread_mutex_lock(&pTask->pThis->m_pmDownload);
	pTask->pThis->m_nThreadCnt--;
	pthread_mutex_unlock(&pTask->pThis->m_pmDownload);

	free(pTask);

	return NULL;
}

bool HttpDownloader::createFile(const std::string &strFile, int nSize)
{
	std::ofstream outStream;
	outStream.open(strFile.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
	outStream.seekp(nSize - 1);
	outStream.put('\0');
	outStream.close();
	return true;
}

