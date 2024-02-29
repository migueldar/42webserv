#ifndef CDCGIHANDLER_HPP
#define CDCGIHANDLER_HPP

#include <iostream>
#include "delete.hpp"
#include <map>

// src/cgi/cgiHandler.cpp
class CgiHandler {
	public:
		using parsePointer = void(CgiHandler::*)();

		enum metaVariables{
			SCRIPT_NAME,
			AUTH_TYPE,
			CONTENT_LENGTH,
			CONTENT_TYPE,
			GATEWAY_INTERFACE,
			PATH_INFO,
			PATH_TRANSLATED,
			QUERY_STRING,
			REMOTE_ADDR,
			REMOTE_HOST,
			REMOTE_IDENT,
			REMOTE_USER,
			REQUEST_METHOD,
			SERVER_NAME,
			SERVER_PORT,
			SERVER_PROTOCOL,
			SERVER_SOFTWARE,
		};

		CgiHandler(std::string pathScript ,Request req, std::vector<std::string> uri, std::string query_string);
		void 	initDictParser(void);

		void	parseSCRIPT_NAME(void);

		void	parseAUTH_TYPE(void);
		void	parseCONTENT_LENGTH(void);
		void	parseCONTENT_TYPE(void);
		void	parseGATEWAY_INTERFACE(void);
		void	parsePATH_INFO(void);
		void	parsePATH_TRANSLATED(void);
		void	parseQUERY_STRING(void);
		void	parseREMOTE_ADDR(void);
		void	parseREMOTE_HOST(void);
		void	parseREMOTE_IDENT(void);
		void	parseREMOTE_USER(void);
		void	parseREQUEST_METHOD(void);
		void	parseSERVER_NAME(void);
		void	parseSERVER_PORT(void);
		void	parseSERVER_PROTOCOL(void);
		void	parseSERVER_SOFTWARE(void);
		
	
	private:
		std::map<enum metaVariables, parsePointer> 	methodMap;
		std::string 								body;
		std::map<std::string, std::string> 			metaVariables;
		Request 									&req;
		std::vector<std::string>					&uri; 
		std::string 								&query_string;
};

#endif