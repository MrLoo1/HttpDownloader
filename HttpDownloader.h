/*
 * HttpDownloader.h
 *
 *  Created on: Apr 20, 2018
 *      Author: pxxian
 */

#ifndef HTTPDOWNLOADER_H_
#define HTTPDOWNLOADER_H_

#include <string>
#include <pthread.h>
#include "HttpClient.h"

class HttpDownloader {
public:
	HttpDownloader();
	virtual ~HttpDownloader();

	/**
	 * downloadUrlToFile
	 * 		download the url to local file.
	 * param:
	 * 		strUrl: http URL
	 * 		strLoadFile: the file path and name
	 *
	 * return: bool
	 * 		whether the download task is create or not.
	 * 		true: created
	 * 		false: failed to create
	 */
	bool downloadUrlToFile(const std::string &strUrl, const std::string &strLoadFile);

	/**
	 * isCompleted
	 * 		whether the download task is completed.
	 * 	param:
	 * 		bResult: return the download status: success or failed.
	 *
	 * 	return: bool
	 * 		whether the download task is completed.
	 * 		true: completed
	 * 		false: incomplete
	 */
	bool isCompleted(bool & bResult);


private:
	/**
	 * leaderThread
	 * 		the thread that guides the downloading task
	 */
	static void* leaderThread(void* param);
	/**
	 * downloadThread
	 * 		the thread that Process the download block task
	 */
	static void* downloadThread(void* param);

	/**
	 * downloadProcess
	 * 		Process the downloading task
	 */
	bool downloadProcess(void);

	/**
	 * createFile
	 * 		create a file with the size of "nSize";
	 *
	 * param:
	 * 		strFile: the file name.
	 * 		nSize: the size of file.
	 *
	 * 	return: bool.
	 * 		whether created or no.
	 */
	bool createFile(const std::string &strFile, int nSize);

	std::string m_strUrl;
	std::string m_strLoadFile;

	bool m_bCompleted;
	bool m_bSuccess;
	bool m_bFailed;

	int m_nThreadCnt;

	pthread_t m_ptLeaderThread;
	pthread_mutex_t m_pmDownload;

	HttpClient* m_httpClient;

};

#endif /* HTTPDOWNLOADER_H_ */
