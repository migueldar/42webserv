#ifndef HTTP_HPP
#define HTTP_HPP

#include "Request.hpp"

bool		isToken(std::string& str);
bool		isSegment(std::string& str);
bool		isFieldLine(std::string& str);
bool		isIPV4(std::string str);
bool		isHostHeader(std::string& str);
bool		isAllDigits(std::string& str);
int			hexToNum(char c);
std::string parsePctEncoding(std::string& str);
std::string	getHTTPLine(std::string::const_iterator& it, std::string::const_iterator& end);

#endif