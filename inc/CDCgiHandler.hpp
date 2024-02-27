#ifndef CDCGIHANDLER_HPP
#define CDCGIHANDLER_HPP

#include <iostream>
#include "delete.hpp"

enum CGI{
	py = 0,
	go = 1,
};

// src/cgi/cgiHandler.cpp
class CgiHandler {
	private:
		enum CGI 	type;
		std::string	body;
		std::string script_name;
		std::string path_info;
		std::string query_string;

	public:
		CgiHandler(Request);

		void setParams(std::string route, std::string params);

};


#endif