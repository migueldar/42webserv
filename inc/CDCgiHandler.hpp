#ifndef CDCGIHANDLER_HPP
#define CDCGIHANDLER_HPP

#include <iostream>
#include "delete.hpp"

enum CGI{
	PY = 0,
	GO = 1,
};

// src/cgi/cgiHandler.cpp
class CgiHandler {
	private:
		enum CGI 	type;
		Request 	*req;
		std::string script_name;
		std::string path_info;
		std::string query_string;

	public:
		CgiHandler(Request *req, std::vector<std::string> uri, std::string query_string, enum CGI type);

};

//Variables de entorno van al execv (meta variables añadidas al **)
//Infile al pipe  del ejecutable pasas body(gestion poll)
//path relativo + script selection
//path relativo + path_info
#endif