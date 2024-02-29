
#include "webserv.hpp"

CgiHandler::CgiHandler(std::string pathScript, Request req, std::vector<std::string> uri, std::string query_string): req(req), uri(uri), query_string(query_string) {
    initDictParser();
    //cgiScrpitSelector
    //
}

void CgiHandler::initDictParser(void){
    methodMap[SCRIPT_NAME] = &CgiHandler::parseSCRIPT_NAME;
}
void CgiHandler::parseLOCATION(void) {
    std::string location;

    while (!uri.empty()) {
        size_t found = uri[0].find(req.cgiToken);
        if (found != std::string::npos) {
            metaVariables["LOCATION"] = location;
            break;
        }
        location += "/" + uri[0];
        uri.erase(uri.begin());
    }
    return;
}

void CgiHandler::parseSCRIPT_NAME(void) {
    bool    bad = 0;


    for (size_t i = 0; i < uri.size(); i++) {
        size_t found = uri[i].find(req.cgiToken);

        if (found != std::string::npos) {
            i += found;
            bad += 1;
            script = uri[i];
            metaVariables["SCRIPT_NAME"] = metaVariables["LOCATION"] + "/" + uri[i];
            uri.erase(uri.begin());
        }
    }
    if(bad != 1){
        //throw MAS DE UN SCRIPT EN URL
    }
    return;
}



void	parsePATH_INFO(void){
    
}
