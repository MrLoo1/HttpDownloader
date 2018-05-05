/*
 * Logger.cpp
 *
 *  Created on: Apr 20, 2018
 *      Author: pxxian
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include "Logger.h"

pthread_mutex_t g_lockLogger = PTHREAD_MUTEX_INITIALIZER;

Logger* Logger::s_logger = NULL;

Logger::Logger()
{

}

Logger::~Logger()
{

}

Logger* Logger::getSingleton()
{
	if (!s_logger)
	{
		pthread_mutex_lock(&g_lockLogger);
		if (!s_logger)
		{
			s_logger = new Logger();
		}
		pthread_mutex_unlock(&g_lockLogger);
	}

	return s_logger;
}

void Logger::log(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	// Now print to the terminal, but you can write to the log file.
	size_t len = strlen(fmt);
	char *tmp = new char[len+2];
	strcpy(tmp, fmt);
	tmp[len] = '\n';
	tmp[len+1] = '\0';

	vprintf(tmp, args);

	delete[] tmp;

	va_end(args);
}
