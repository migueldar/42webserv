
#include "Response.hpp"
#include "http.hpp"
#include "CDCgiHandler.hpp"

std::string Response::getHttpResponse(){
    return httpResponse;
}

Response::Response(std::string port, const Server& server, Request req): header(""), body(""), httpResponse(""), reconstructPath(reconstructPathFromVec(req.target)), loc(getLocationByRoute(reconstructPath, server)), newCgi(NULL), req(req), cgiToken(""), port(port), status(START_PREPING_RES) {
    // TODO: remove hardcode
    httpResponse = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: keep-alive\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n";
}

Response::~Response(){
}

const Location& Response::getLocationByRoute(std::string reconstructedPath, const Server& server) {
    std::string remainingPath = reconstructedPath;
    while (42) {
        try {
			if(server.existsLocationByRoute(remainingPath)){
                locationPath = remainingPath;
            	return server.getLocation(remainingPath);
            }
        } catch (const std::out_of_range&) {
			//JUST TO CATH WHEN GET LOCATION STD::MAP "AT" METHOD DOESNT FIND REQUESTED LINE 
        }

		if(remainingPath == "")
			break;

        // BREAKING PATHS BY '/'
		size_t lastSlashPos = remainingPath.rfind('/');
		if (lastSlashPos == std::string::npos)
			remainingPath = "";
		else
			remainingPath = remainingPath.substr(0, lastSlashPos);
    }
    //EMPTY LOCATION TO RETURN 404 ERROR, STATIC TO RETURN SAME OBJECT ALWAIS
    static Location locDef;
	return locDef;
}

int Response::prepareResponse() {
    int ret;
    std::string auxTest = loc.root + reconstructPath.substr(locationPath.size());

    switch (status) {
    case START_PREPING_RES:
        return handleStartPrepingRes(auxTest);

    case WAITING_FOR_CGI:
        ret = handleWaitingForCgi();
        if(ret == -1)
            return 0;
        return ret;

    case PROCESSING_RES: // TODO: Read from file opened and storage
        status = GET_RESPONSE;
        return 0;

    case GET_RESPONSE:
        handleGetResponse();
        break;
    }

    return -1;
}

int Response::handleStartPrepingRes(const std::string& auxTest) {
    status = PROCESSING_RES;

    // IF WE HAVE ACCESS TO FILE AND LOCATION ISNT A DEFAULT ONE
    if (checkAccess(auxTest) && loc.defaultPath != "" && loc.root != "" && loc.redirectionUrl != "") {
        
        //CHECKING PATH FOR CGI TOKENS REGISTERED
        for (std::map<std::string, std::string>::const_iterator it = loc.cgi.begin(); it != loc.cgi.end(); ++it) {
            if (std::find(req.target.begin(), req.target.end(), it->first) != req.target.end()) {
                cgiToken = it->first;
                break;
            }
        }

        //CREATING CGI HANDLER
        if (!cgiToken.empty()) {
            newCgi = new CgiHandler(loc, cgiToken, port, req, req.target, req.queryParams);
            status = WAITING_FOR_CGI;
        }
        else{
            // TODO: Open the file and return the file descriptor for poll insertion
        }
    } else {
        handleFileNotFound();
    }

    return 0;
}

int Response::handleWaitingForCgi() {
    int fdRet = newCgi->handleCgiEvent();

    if (fdRet != -1) 
        return fdRet;
    
    status = GET_RESPONSE;
    return -1;
}

void Response::handleGetResponse() {
    if (!cgiToken.empty()) {
        body = newCgi->getCgiResponse();
        std::cout << body << std::endl;
        delete newCgi;
    }
    
    // TODO: all remaining response build
}

void Response::handleFileNotFound() {
    // Add checking of default pages here
    httpResponse = "HTTP/1.1 404 OK\r\nContent-Length: 0\r\nConnection: keep-alive\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n";
    status = GET_RESPONSE;
}
