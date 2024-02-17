#ifndef CDCGIHANDLER_HPP
#define CDCGIHANDLER_HPP

#include <iostream>

enum CGI{
	py = 0,
	go = 1,
};

// src/cgi/cgiHandler.cpp
class CgiHandler {
	private:
		enum CGI type;
		std::string exec;
		std::string route;
		std::string params;

	public:
		CgiHandler(enum CGI type);

		void setParams(std::string route, std::string params);

};


#endif