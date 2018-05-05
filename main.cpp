/*
 * main.cpp
 *
 *  Created on: Apr 20, 2018
 *      Author: pxxian
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "Logger.h"
#include "HttpDownloader.h"


int main(int argc, char** argv)
{
	Logger::getSingleton()->log("Info: downloader start ...");

	if (argc < 3)
	{
		printf("Usage: ./downloader url local\n"
				"e.g. ./downloader http://nginx.org/download/nginx-1.14.0.tar.gz ./nginx.tar.gz\n"
				"    url: the http url\n"
				"  local: the local file name\n");
		return -1;
	}

	// Http multi-thread Downloader
	HttpDownloader downloader;
	downloader.downloadUrlToFile(argv[1], argv[2]);
	bool bRet;
	// Is it done?
	while (!downloader.isCompleted(bRet))
	{
		sleep(1);
	}
	// is it success?
	if (bRet)
	{
		Logger::getSingleton()->log("Info: download file success");
	}
	else
	{
		Logger::getSingleton()->log("Info: download file failed");
	}

	return 0;
}



