#ifndef CDCGIHANDLER_HPP
#define CDCGIHANDLER_HPP

#include <iostream>
#include "delete.hpp"
#include <map>

// src/cgi/cgiHandler.cpp
class CgiHandler {
	public:

		enum metaVariables{
			LOCATION,
			SCRIPT_NAME,
			PATH_INFO,
			PATH_TRANSLATED,
			QUERY_STRING,
			AUTH_TYPE,
			CONTENT_LENGTH,
			CONTENT_TYPE,
			GATEWAY_INTERFACE,
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

		CgiHandler(Location loc, std::string tokenCGI ,Request req, std::vector<std::string> uri, std::string query_string);
		void 	initDictParser(void);

		void	parseLOCATION(void);
		void	parseSCRIPT_NAME(void);
		void	parsePATH_INFO(void);
		void	parsePATH_TRANSLATED(void);
		void	parseQUERY_STRING(void);
		void	parseREQUEST_METHOD(void);

		void	parseAUTH_TYPE(void);
		void	parseCONTENT_LENGTH(void);
		void	parseCONTENT_TYPE(void);
		void	parseGATEWAY_INTERFACE(void);
		void	parseREMOTE_ADDR(void);
		void	parseREMOTE_HOST(void);
		void	parseREMOTE_IDENT(void);
		void	parseREMOTE_USER(void);
		void	parseSERVER_NAME(void);
		void	parseSERVER_PORT(void);
		void	parseSERVER_PROTOCOL(void);
		void	parseSERVER_SOFTWARE(void);
		
	private:
		std::string												tokenCGI;
		std::map<enum metaVariables, void(CgiHandler::*)()> 	methodMap;
		std::string 											body;
		std::map<std::string, std::string>		 				metaVariables;
		std::string												script;
		Request 												&req;
		std::vector<std::string>								&uri; 
		std::string 											&query_string;
		Location 												&loc;
};

#endif