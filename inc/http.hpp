#ifndef HTTP_HPP
#define HTTP_HPP

#include "Request.hpp"

bool	isToken(std::string str);
bool	isSegment(std::string str);
int		hexToNum(char c);

#endif