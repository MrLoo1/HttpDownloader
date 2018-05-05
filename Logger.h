/*
 * Logger.h
 *
 *  Created on: Apr 20, 2018
 *      Author: pxxian
 */

#ifndef LOGGER_H_
#define LOGGER_H_

class Logger {
public:
	Logger();
	virtual ~Logger();

	void log(const char *fmt, ...);

	static Logger* getSingleton();

private:
	static Logger *s_logger;
};

#endif /* LOGGER_H_ */
