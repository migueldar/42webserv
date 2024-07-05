#ifndef CDCGIHANDLER_HPP
#define CDCGIHANDLER_HPP

#include <iostream>
#include "Request.hpp"
#include "parser.hpp"
#include <map>

#define METAVARIABLES_LENGTH 15

// src/cgi/cgiHandler.cpp
class CgiHandler {
	public:

		enum CGI_STAGES {
			BEGIN_CGI_EXEC = 0,
			WRITE_CGI_EXEC,
			READ_CGI_EXEC,
		};
		enum metaVariables{
			LOCATION = 0,
			SCRIPT_NAME,
			PATH_INFO,
			PATH_TRANSLATED,
			QUERY_STRING,
			REQUEST_METHOD,
			SERVER_PROTOCOL,
			SERVER_NAME,
			REMOTE_ADDR,
			SERVER_PORT,
			SERVER_SOFTWARE,
			AUTH_TYPE,
			CONTENT_LENGTH,
			CONTENT_TYPE,
			GATEWAY_INTERFACE,			
			REMOTE_USER,
		};
		
		
		CgiHandler(const Location &loc, std::string &tokenCGI, std::string &port, Request &req, std::vector<std::string> &uri, std::string &query_string);
		CgiHandler(const CgiHandler& other);
		~CgiHandler();
		void 	initDictParser(void);
		long 	handleCgiEvent();
		int 	executeCgi(int &fd);

		void	parseLOCATION(void);
		void	parseSCRIPT_NAME(void);
		void	parsePATH_INFO(void);
		void	parsePATH_TRANSLATED(void);
		void	parseQUERY_STRING(void);
		void	parseREQUEST_METHOD(void);
		void	parseSERVER_PROTOCOL(void);
		void	parseSERVER_NAME(void);
		void	parseREMOTE_ADDR(void);
		void	parseSERVER_PORT(void);
		void	parseSERVER_SOFTWARE(void);
		void	parseAUTH_TYPE(void);
		void	parseCONTENT_LENGTH(void);
		void	parseCONTENT_TYPE(void);
		void	parseGATEWAY_INTERFACE(void);
		void	parseREMOTE_IDENT(void);
		void	parseREMOTE_USER(void);

		std::string getCgiResponse() const ;
		
	private:
		std::string												tokenCGI;
		std::map<enum metaVariables, void(CgiHandler::*)()> 	methodMap;
		std::string 											body;
		std::map<std::string, std::string>		 				metaVariables;
		std::string												script;
		std::string												&port;
		Request 												&req;
		std::vector<std::string>								&uri; 
		std::string 											&query_string;
		const Location 											&loc;
		char 													**env;

		std::string 											response;
};

#endif