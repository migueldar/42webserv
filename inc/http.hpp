#ifndef HTTP_HPP
#define HTTP_HPP

#include "Request.hpp"

bool		isToken(std::string str);
bool		isSegment(std::string str);
bool		isFieldLine(std::string str);
int			hexToNum(char c);
std::string	getHTTPLine(std::string::const_iterator& it, std::string::const_iterator& end);

#endif