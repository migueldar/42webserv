
#include "webserv.hpp"

std::string setExex(enum CGI type){
	std::string exec;

	switch (type){
		case py:
			exec = "python3 ";
			break;
		case go:
			exec = "go run ";
			break;
	}

	return(exec);
}

CgiHandler::CgiHandler(enum CGI type): type(type){
	exec = setExec();
}

void CgiHandler::setParams(std::string route, std::string params){
	this->route = route;
	this->params = params;
}