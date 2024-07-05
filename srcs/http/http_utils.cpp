#include <iostream>
#include <algorithm>
#include "Request.hpp"
#include "Response.hpp"
#include "http.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "unistd.h"

//defined in RFC 9110
bool isToken(std::string& str) {
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
bool isSegment(std::string& str) {
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
bool isFieldLine(std::string& str) {
	if (str.length() == 0 || (str.length() == 1 && (unsigned char) str[0] >= 0x21))
		return true;
	if (!((unsigned char) str[0] >= 0x21 && (unsigned char) str[str.length() - 1] >= 0x21))
		return false;
	for (std::string::const_iterator it = str.begin() + 1; it != str.end() - 1; it++)
		if (!((unsigned char) *it >= 0x20 || *it == '\t'))
			return false;
	return true;
}

//defined in RFC 3986
bool isQuery(std::string str) {
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
		if (*it == '%') {
			if (!isxdigit(it[1]) || !isxdigit(it[2]))
				return false;
			it++; it++;
		}
		else if (!(isalnum(*it) || *it == '-' || *it == '.' || *it == '_' || *it == '~'\
				|| *it == '!' || *it == '$' || *it == '&' || *it == '\'' || *it == '('\
				|| *it == ')' || *it == '*' || *it == '+' || *it == ',' || *it == ';'\
				|| *it == '=' || *it == ':' || *it == '@' || *it == '/' || *it == '?'))
			return false;
	return true;
}

std::string parsePctEncoding(std::string str) {
	std::string target;

	for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
		if (*it == '%') {
			target += hexToNum(it[1]) * 0x10 + hexToNum(it[2]);
			it++; it++;
		}
		else
			target += *it;
	}
	return target;
}

bool isIPV4(std::string str) {
	size_t		found;
	size_t		prev_found = 0;
	std::string	aux;

	for (int i = 0; i < 4; i++) {
		found = str.find(".", prev_found);
		aux = str.substr(prev_found, found - prev_found);
		if (aux.length() > 3 || !isAllDigits(aux) || std::atoi(aux.c_str()) > 255 || std::atoi(aux.c_str()) < 0)
			return false;
		prev_found = found + 1;
	}
	return true;
}

static bool isRegName(std::string& str) {
	if (str.length() == 0)
		return false;
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
		if (*it == '%') {
			if (!isxdigit(it[1]) || !isxdigit(it[2]))
				return false;
			it++; it++;
		}
		else if (!(isalnum(*it) || *it == '-' || *it == '.' || *it == '_' || *it == '~'\
				|| *it == '!' || *it == '$' || *it == '&' || *it == '\'' || *it == '('\
				|| *it == ')' || *it == '*' || *it == '+' || *it == ',' || *it == ';'\
				|| *it == '='))
			return false;
	return true;
}

//defined in RFC 3986, without IP-literal
bool isHost(std::string str) {
	return (isIPV4(str) || isRegName(str));
}

bool isPort(std::string str) {
	return !(str.length() == 0 || str.length() > 5 || !isAllDigits(str) || std::atoi(str.c_str()) > 0xffff || std::atoi(str.c_str()) <= 0);
}

bool isHostHeader(std::string& str) {
	size_t	found;

	found = str.find(":");
	if (found != std::string::npos)
		return (isHost(str.substr(0, found)) && isPort(str.substr(found + 1)));
	else
		return isHost(str);
}

bool isAllDigits(std::string& str) {
	if (str.length() == 0)
		return false;
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
		if (!isdigit(*it))
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

std::string toLower(const std::string &str) {
	std::string ret = "";
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
		ret += tolower(*it);
	return ret;
}

long hexStringToLong(std::string str) {
    std::istringstream iss(str);
    long ret;

    if (!(iss >> std::hex >> ret)) 
		return ret;
    return ret;
}

const Server& getServerByHost(const std::vector<Server>& servers, std::string host) {
    std::vector<Server>::const_iterator it;

    for (it = servers.begin(); it != servers.end(); ++it) {
        if (it->serverName == host) {
            return *it;
        }
    }
	throw (Response::NotFoundException());
}


Response::statusCode checkAccess(const std::string& path, enum methodsEnum method, bool autoIndex) {
    struct stat fileStat;
    
    if (stat(path.c_str(), &fileStat) != 0) {
        return Response::_4XX; 
    }
    
    if (S_ISDIR(fileStat.st_mode)) {
		if(!autoIndex)
        	return Response::_4XX;
		else if(method != GET)
			return Response::_4XX;
	}	

	switch (method){
		case GET:
			if (access(path.c_str(), R_OK) == 0) {
				return Response::_2XX; 
			}
			return Response::_4XX;
		case POST:
			if (access(path.c_str(), W_OK) == 0) {
				return Response::_2XX; 
			}
			return Response::_4XX;
		case DELETE:
			if (access(path.c_str(), W_OK) == 0) {
				return Response::_2XX; 
			}
			return Response::_4XX;
	}
	return Response::_4XX;

}

std::string reconstructPathFromVec(const std::vector<std::string>& pathSplitted){
	std::string reconstructedPath = "/";
    if (!pathSplitted.empty()) {
        for (size_t i = 0; i < pathSplitted.size(); ++i) {
            reconstructedPath += pathSplitted[i];
			if(i < pathSplitted.size() - 1)
				reconstructedPath += "/";
		}
    }
	 
	return(reconstructedPath);
}
