#ifndef HTTP_HPP
#define HTTP_HPP

#include "Request.hpp"
#include "Response.hpp"

bool		isToken(std::string& str);
bool		isSegment(std::string& str);
bool		isFieldLine(std::string& str);
bool		isQuery(std::string str);
bool		isIPV4(std::string str);
bool		isHostHeader(std::string& str);
bool		isAllDigits(std::string& str);
int			hexToNum(char c);
std::string parsePctEncoding(std::string str);
std::string	getHTTPLine(std::string::const_iterator& it, std::string::const_iterator& end);
std::string toLower(const std::string &str);
long		hexStringToLong(std::string str);

const Server &getServerByHost(const std::vector<Server> &servers, std::string host);
const Location& getLocationByRoute(std::string reconstructedPath, const Server& server);
std::string reconstructPathFromVec(const std::vector<std::string>& pathSplitted);

std::string readFile(int fd);

#endif