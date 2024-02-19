#include <iostream>
#include "Request.hpp"

//defined in RFC 9110
bool isToken(std::string str) {
	if (str.length() == 0)
		return false;
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
		if (!(isalnum(*it) || *it == '!' || *it == '#' || *it == '$' || *it == '%'\
			|| *it == '&' || *it == '\'' || *it == '*' || *it == '+' || *it == '-'\
			|| *it == '.' || *it == '^'  || *it == '_' || *it == '`' || *it == '|' || *it == '~'))
			return false;
	return true;
}

//defined in RFC 3986
bool isSegment(std::string str) {
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
		if (*it == '%') {
			if (!isxdigit(it[1]) || !isxdigit(it[2]))
				return false;
			it++; it++;
		}
		else if (!(isalnum(*it) || *it == '-' || *it == '.' || *it == '_' || *it == '~'\
				|| *it == '!' || *it == '$' || *it == '&' || *it == '\'' || *it == '('\
				|| *it == ')' || *it == '*' || *it == '+' || *it == ',' || *it == ';'\
				|| *it == '=' || *it == ':' || *it == '@'))
			return false;
	return true;
}

//defined in RFC 3986
bool isFieldLine(std::string str) {
	if (str.length() == 0 || (str.length() == 1 && (unsigned char) str[0] >= 0x21))
		return true;
	if (!((unsigned char) str[0] >= 0x21 && (unsigned char) str[str.length() - 1] >= 0x21))
		return false;
	for (std::string::const_iterator it = str.begin() + 1; it != str.end() - 1; it++)
		if (!((unsigned char) *it >= 0x20 || *it == '\t'))
			return false;
	return true;
}

int hexToNum(char c) {
	if (isdigit(c))
		return c - '0';
	if (islower(c))
		return c - 'a' + 10;
	return c - 'A' + 10;
}

bool isAllDigits(std::string str) {
	if (str.length() == 0)
		return false;
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
		if (!isdigit(*it))
			return false;
	return true;
}

// its normal line but ends in \r\n
std::string getHTTPLine(std::string::const_iterator& it, std::string::const_iterator& end) {
	std::string ret;

	while (it != end && it + 1 != end && *it != '\r' && *(it + 1) != '\n') {
		ret += *it;
		it++;
	}
	if (it == end || it + 1 == end)
		throw Request::BadRequest();
	it++;it++;
	return ret;
}

//status 1 if fail
// int	ft_atoi(const char *str, int *status)
// {
// 	int				minus;
// 	long int		ret;
// 	int				counter;

// 	ret = 0;
// 	minus = 0;
// 	counter = 0;
// 	if (*str == '+' || *str == '-')
// 		if (*(str++) == '-')
// 			minus++;
// 	while (*str)
// 	{
// 		if (!(*str <= '9' && *str >= '0'))
// 			return (*status = 0, 0);
// 		ret = ret * 10 + (*str++ - '0');
// 		counter++;
// 	}
// 	if (counter > 10 || (ret > 0x7fffffff && !minus)
// 		|| (ret > 0x80000000 && minus))
// 		return (*status = 1, 0);
// 	if (minus)
// 		ret *= -1;
// 	return (*status = 0, ret);
// }

