
#include "webserv.hpp"

CgiHandler::CgiHandler(Location loc, std::string tokenCGI ,Request req, std::vector<std::string> uri, std::string query_string): tokenCGI(tokenCGI), req(req), uri(uri), query_string(query_string), loc(loc){
    initDictParser();
    //cgiScrpitSelector
    //
}

// server {

//     location / {
//         root /usr/local/www/htdocs
//     }

//     location /32313 {
//         root /usr/local/www/htdocs
//     }
// }

// http://somehost.com/32313/app.py/this.is.the.path%3binfo

// /32313/this.is.the.path%3binfo

// /usr/local/www/htdocs/32313/this.is.the.path%3binfo

// /usr/local/www/htdocs/this.is.the.path%3binfo



void CgiHandler::initDictParser(void){
    methodMap[LOCATION] = &CgiHandler::parseLOCATION;
    methodMap[SCRIPT_NAME] = &CgiHandler::parseSCRIPT_NAME;
    methodMap[PATH_INFO] = &CgiHandler::parsePATH_INFO;
    methodMap[PATH_TRANSLATED] = &CgiHandler::parsePATH_TRANSLATED;
    methodMap[QUERY_STRING] = &CgiHandler::parseQUERY_STRING;
    methodMap[REQUEST_METHOD] = &CgiHandler::parseREQUEST_METHOD;
}

void CgiHandler::parseLOCATION(void) {
    std::string location = "";

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
    }
    return;
}



void	CgiHandler::parsePATH_INFO(void){
    std::string pathinfo = "";
    int         found = 0;

    while (!uri.empty()) {
        pathinfo += "/" + uri[0];
        uri.erase(uri.begin());
    }
    metaVariables["PATH_INFO"] += "/" + pathinfo;
    metaVariables["PATH_TRANSLATED"] += "/" + pathinfo;
    return;
}

void	CgiHandler::parsePATH_TRANSLATED(void){

    metaVariables["PATH_TRANSLATED"] = "/" + loc.root + metaVariables["PATH_TRANSLATED"];
    return;
}

void	CgiHandler::parseQUERY_STRING(void){
    metaVariables["QUERY_STRING"] = query_string;
    return;
}

void	CgiHandler::parseREQUEST_METHOD(void){
    //metaVariables["reqparseREQUEST_METHOD"] = req.;
    return;
}