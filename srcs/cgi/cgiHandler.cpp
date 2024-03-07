
#include "webserv.hpp"

CgiHandler::CgiHandler(Location &loc, std::string &tokenCGI, std::string &port, Request &req, std::vector<std::string> &uri, std::string &query_string): tokenCGI(tokenCGI), port(port), req(req), uri(uri), query_string(query_string), loc(loc){
    initDictParser();
    for (enum metaVariables x = LOCATION; x < METAVARIABLES_LENGTH; x = static_cast<enum metaVariables>(x + 1)) {
        (this->*methodMap[x])();
    }
    for (std::map<std::string, std::string>::iterator it = metaVariables.begin(); it != metaVariables.end(); it++){
        std::cout << it->first << "=" << it->second << std::endl;
    }
}

CgiHandler::~CgiHandler(){
}

void CgiHandler::initDictParser(void){
    methodMap[LOCATION] = &CgiHandler::parseLOCATION;
    methodMap[SCRIPT_NAME] = &CgiHandler::parseSCRIPT_NAME;
    methodMap[PATH_INFO] = &CgiHandler::parsePATH_INFO;
    methodMap[PATH_TRANSLATED] = &CgiHandler::parsePATH_TRANSLATED;
    methodMap[QUERY_STRING] = &CgiHandler::parseQUERY_STRING;
    methodMap[REQUEST_METHOD] = &CgiHandler::parseREQUEST_METHOD;
    methodMap[SERVER_PROTOCOL] = &CgiHandler::parseSERVER_PROTOCOL;
    methodMap[SERVER_NAME] = &CgiHandler::parseSERVER_NAME;
    methodMap[SERVER_PORT] = &CgiHandler::parseSERVER_PORT;
    methodMap[SERVER_SOFTWARE] = &CgiHandler::parseSERVER_SOFTWARE;
    methodMap[AUTH_TYPE] = &CgiHandler::parseAUTH_TYPE;
    methodMap[CONTENT_LENGTH] = &CgiHandler::parseCONTENT_LENGTH;
    methodMap[CONTENT_TYPE] = &CgiHandler::parseCONTENT_TYPE;
    methodMap[GATEWAY_INTERFACE] = &CgiHandler::parseGATEWAY_INTERFACE;
}

void CgiHandler::parseLOCATION(void) {
    std::string location = "";
    std::cout << LOCATION << std::endl;

    while (!uri.empty()) {
        size_t found = uri[0].find(tokenCGI);
        if (found != std::string::npos) {
            metaVariables["LOCATION"] = location;
            break;
        }
        location += "/" + uri[0];
        uri.erase(uri.begin());
    }
    if(location == ""){
        metaVariables["LOCATION"] = "/";
    }
    return;
}

void CgiHandler::parseSCRIPT_NAME(void) {
    bool    bad = 0;
    std::cout << SCRIPT_NAME << std::endl;

    for (size_t i = 0; i < uri.size(); i++) {
        size_t found = uri[i].find(tokenCGI);

        if (found != std::string::npos) {
            i += found;
            bad += 1;
            script = uri[i];
            metaVariables["SCRIPT_NAME"] = metaVariables["LOCATION"] + "/" + script;
            metaVariables["PATH_INFO"] = "/" + script;
            metaVariables["PATH_TRANSLATED"] = "/" + script;
            uri.erase(uri.begin());
        }
    }
    if(bad != 1){
        //throw MAS DE UN SCRIPT EN URL
        return;
    }
    return;
}



void	CgiHandler::parsePATH_INFO(void){
    std::string pathinfo = "";
    std::cout << PATH_INFO << std::endl;

    while (!uri.empty()) {
        pathinfo += "/" + uri[0];
        uri.erase(uri.begin());
    }
    metaVariables["PATH_INFO"] += "/" + pathinfo;
    metaVariables["PATH_TRANSLATED"] += "/" + pathinfo;
    return;
}

void	CgiHandler::parsePATH_TRANSLATED(void){
    std::cout << PATH_TRANSLATED << std::endl;
    metaVariables["PATH_TRANSLATED"] = "/" + loc.root + metaVariables["PATH_TRANSLATED"];
    return;
}

void	CgiHandler::parseQUERY_STRING(void){
    std::cout << QUERY_STRING << std::endl;
    metaVariables["QUERY_STRING"] = query_string;
    return;
}

void	CgiHandler::parseREQUEST_METHOD(void){
    std::cout << REQUEST_METHOD << std::endl;
    switch (req.method)
    {
    case GET:
        metaVariables["REQUEST_METHOD"] = "GET";
        break;
    case DELETE:
        metaVariables["REQUEST_METHOD"] = "DELETE";
        break;
    case POST:
        metaVariables["REQUEST_METHOD"] = "POST";
        break;
    }
    return;
}

void	CgiHandler::parseSERVER_PROTOCOL(void){
    std::cout << SERVER_PROTOCOL << std::endl;
    metaVariables["SERVER_PROTOCOL"] = "HTTP";
    return;
}

void	CgiHandler::parseSERVER_NAME(void){
    std::cout << SERVER_NAME << std::endl;
    std::cout << "PETO" << std::endl;
    metaVariables["SERVER_NAME"] = req.headers["Host"];
    return;
}

void    CgiHandler::parseREMOTE_ADDR(void){
    std::cout << REMOTE_ADDR << std::endl;
    metaVariables["REMOTE_ADDR"] = req.address;
    return;
}

void    CgiHandler::parseSERVER_PORT(void){
    std::cout << SERVER_PORT << std::endl;
    metaVariables["SERVER_PORT"] = port;
    return;
}

void	CgiHandler::parseSERVER_SOFTWARE(void){
    std::cout << SERVER_SOFTWARE << std::endl;
    metaVariables["SERVER_SOFTWARE"] = "1.1";
    return;
}

void	CgiHandler::parseAUTH_TYPE(void){
    std::cout << AUTH_TYPE << std::endl;
    if(req.headers["auth-scheme"] != ""){
        metaVariables["AUTH_TYPE"] = req.headers["auth-scheme"];
    }
    return;
}

void	CgiHandler::parseCONTENT_LENGTH(void){
    std::cout << CONTENT_LENGTH << std::endl;
    if(req.body != ""){
        metaVariables["CONTENT_LENGTH"] = std::to_string(req.body.length());
    }
    return;
}

void	CgiHandler::parseCONTENT_TYPE(void){
    std::cout << CONTENT_TYPE << std::endl;
    if(req.headers["Content-Type"] != ""){
        metaVariables["CONTENT_TYPE"] = req.headers["Content-Type"];
    }
    return;
}

void	CgiHandler::parseGATEWAY_INTERFACE(void){
    metaVariables["GATEWAY_INTERFACE"] = "CGI / 1.1";
    return;
}

void CgiHandler::parseREMOTE_USER(void){
    if(metaVariables["AUTH_TYPE"] != ""){
        metaVariables["REMOTE_USER"] = req.headers["Host"];
    }
    return;
}
