
#include "webserv.hpp"

CgiHandler::CgiHandler(std::string pathScript, Request req, std::vector<std::string> uri, std::string query_string){
    initDictParser();
    //cgiScrpitSelector
    //
}

void CgiHandler::initDictParser(void){
    methodMap[SCRIPT_NAME] = &CgiHandler::parseSCRIPT_NAME;
}

void	parseSCRIPT_NAME(void)