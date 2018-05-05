/*
 * Utils.h
 *
 *  Created on: Apr 20, 2018
 *      Author: pxxian
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
/**
 * trim
 * 		trim string leading and trailing white spaces
 */
std::string &trim(std::string & s);

/**
 * base64 encode end decode
 */
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int len);
std::string base64_decode(std::string const& s);

#endif /* UTILS_H_ */
