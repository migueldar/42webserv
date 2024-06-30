
#include "Response.hpp"
#include "http.hpp"
#include "CDCgiHandler.hpp"

std::string Response::getHttpResponse(){
    return httpResponse;
}

// TODO: Store server for errorPages, isnt stored now becouse isnt needed
Response::Response(std::string port, const Server& server, Request req): header(""), body(""), httpResponse(""), reconstructPath(reconstructPathFromVec(req.target)), loc(getLocationByRoute(reconstructPath, server)), newCgi(NULL), req(req), cgiToken(""), port(port), status(START_PREPING_RES) {
    // TODO: remove hardcode

    std::cout << "ENtro" << std::endl;
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

/**
 * @brief Prepares the response based on the current response status.
 *
 * This function handles the preparation of the response based on the current status
 * of the Response object. It performs different actions depending on the status,
 * such as starting the preparation process, handling CGI events, processing responses,
 * or returning the response.
 * 
 * Its designed like a finite  state machine becouse recurrent returns of file descriptors are needed to be insert into poll
 *
 * @return 0 if can continue to the next stage, -1 if finishes processing, anything else are fds to be imputed into poll
 */
int Response::prepareResponse() {
    int ret;
    std::string auxTest = loc.root + reconstructPath.substr(locationPath.size());

    switch (status) {
    case START_PREPING_RES:
        return handleStartPrepingRes(auxTest);

    case WAITING_FOR_CGI:
        ret = handleWaitingForCgi();
        if (ret == -1)
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

/**
 * @brief Handles the start of preparing the response.
 *
 * This function sets the status to PROCESSING_RES and checks if access
 * to the file is allowed and if the location is not a default one. It
 * also checks for CGI tokens registered in the location path, creates
 * a CGI handler if a token is found, and sets the status to WAITING_FOR_CGI.
 * If no CGI token is found, it may open the file and return the file descriptor
 * for poll insertion in future implementations.
 *
 * @param auxTest The auxiliary test string used to check machine route to request target.
 * @return Always returns 0 to continue state machine (indicators for future implementation).
 */
int Response::handleStartPrepingRes(const std::string& auxTest) {
    status = PROCESSING_RES;

    // Check access to file and non-default location
    if (checkAccess(auxTest) && !loc.defaultPath.empty() && !loc.root.empty() && !loc.redirectionUrl.empty()) {
        
        // Check for CGI tokens in the path
        for (std::map<std::string, std::string>::const_iterator it = loc.cgi.begin(); it != loc.cgi.end(); ++it) {
            if (std::find(req.target.begin(), req.target.end(), it->first) != req.target.end()) {
                cgiToken = it->first;
                break;
            }
        }

        // Create CGI handler if CGI token found
        if (!cgiToken.empty()) {
            newCgi = new CgiHandler(loc, cgiToken, port, req, req.target, req.queryParams);
            status = WAITING_FOR_CGI;
        }
        else {
            // TODO: Open the file and return the file descriptor for poll insertion
        }
    } else {
        handleFileNotFound();
    }

    return 0;
}

/**
 * @brief Handles waiting for CGI processing to complete.
 *
 * This function calls the handleCgiEvent() method on the newCgi object
 * to process CGI events. If the event handling returns a valid file descriptor,
 * it is returned; When finished processing CGI returns -1
 *
 * @return File descriptor returned by CGI event handling, or -1 if finished.
 */
int Response::handleWaitingForCgi() {
    int fdRet = newCgi->handleCgiEvent();

    if (fdRet != -1) 
        return fdRet;
    
    status = GET_RESPONSE;
    return -1;
}

/**
 * @brief Handles retrieving the response after CGI processing.
 *
 * This function retrieves the CGI response body if a CGI token was found.
 * It also prints the body to standard output and deletes the CGI handler object.
 * Further implementation should handle the remaining response build tasks.
 */
void Response::handleGetResponse() {
    if (!cgiToken.empty()) {
        body = newCgi->getCgiResponse();
        std::cout << body << std::endl;
        delete newCgi;
    }
    
    // TODO: Handle all remaining response build tasks
}

/**
 * @brief Handles the scenario where the requested file or location is not found.
 *
 * This function generates a standard HTTP 404 response indicating that the requested
 * resource could not be found. It sets the httpResponse string accordingly and updates
 * the status to GET_RESPONSE for further response handling.
 */
void Response::handleFileNotFound() {
    // TODO: Add checking of default pages here
    
    httpResponse = "HTTP/1.1 404 OK\r\nContent-Length: 0\r\nConnection: keep-alive\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n";
    status = GET_RESPONSE;
}