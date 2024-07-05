
#include "Response.hpp"
#include "http.hpp"
#include "CDCgiHandler.hpp"
#include <fcntl.h>

std::string Response::getHttpResponse(){
    return httpResponse;
}

// TODO: Store server for errorPages, isnt stored now becouse isnt needed
Response::Response(std::string port, const Server& server, Request req): header(""), body(""), httpResponse(""), reconstructPath(req.targetString), locationPath(""),  cgiToken(""), port(port), loc(getLocationByRoute(reconstructPath, server)), newCgi(NULL), req(req), status(START_PREPING_RES), statusCodeVar(Response::_2XX) {
    // TODO: remove hardcode
    httpResponse = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: keep-alive\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n";
}

// Response::Response(const Response& other)
//     : header(other.header), body(other.body), httpResponse(other.httpResponse), reconstructPath(other.reconstructPath), locationPath(other.locationPath), loc(other.loc), newCgi(NULL), req(other.req), cgiToken(other.cgiToken), port(other.port), status(other.status) {
//     if (other.newCgi != NULL) {
//         newCgi = new CgiHandler(*other.newCgi);
//     }
// }

// Response& Response::operator=(const Response& other) {
//     if (this != &other) {
//         // Liberar recursos existentes
//         if (newCgi != NULL) {
//             delete newCgi;
//             newCgi = NULL;
//         }

//         // Copiar los recursos del objeto other
//         header = other.header;
//         body = other.body;
//         httpResponse = other.httpResponse;
//         reconstructPath = other.reconstructPath;
//         locationPath = other.locationPath;
//         req = other.req;
//         cgiToken = other.cgiToken;
//         port = other.port;
//         status = other.status;

//         if (other.newCgi != NULL) {
//             newCgi = new CgiHandler(*other.newCgi);
//         } else {
//             newCgi = NULL;
//         }
//     }
//     return *this;
// }

Response::~Response(){
}

const Location& Response::getLocationByRoute(std::string reconstructedPath, const Server& server) {
    
    std::string remainingPath = reconstructedPath;
    size_t lastSlashPos = remainingPath.size();
    bool token = 1;
    while (42) {
        // BREAKING PATHS BY '/'
        try {
            if(server.existsLocationByRoute(remainingPath)){
                this->locationPath = std::string(remainingPath);
                return server.getLocation(remainingPath);
            }
        } catch (const std::out_of_range&) {
            std::cout << "NOT FOUND LOCATION " << std::endl; 
			//JUST TO CATH WHEN GET LOCATION STD::MAP "AT" METHOD DOESNT FIND REQUESTED LINE 
        }

		if(remainingPath == "/")
			break;

		lastSlashPos = remainingPath.rfind('/');
        remainingPath = remainingPath.substr(0, lastSlashPos + token);

        if(!token)
            token = 1;
        else
            token = 0;

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
long Response::prepareResponse(int err) {
    long ret;

    if(err){
        statusCodeVar = Response::_4XX; //INTERNAL SERVER ERROR
        status = ERROR_RESPONSE;
    }

    if(status == START_PREPING_RES){
        ret = handleStartPrepingRes();
        
        if(ret != 0)
            return ret;
    }

    if(status == WAITING_FOR_CGI){
        ret = handleWaitingForCgi();

        if (ret != 0)
            return ret;

    } else if(status == PROCESSING_RES){
        status = GET_RESPONSE;
    }

    if(status == GET_RESPONSE){
        handleGetResponse();

    } else if(status == ERROR_RESPONSE){
        ret = handleBadResponse();

        if (ret != 0)
            return ret;
    }

    return -1;
}

Response::statusCode Response::filterResponseCode(const std::string& path, methodsEnum method, bool autoIndex) {
    if (checkAccess(path, method, autoIndex) != Response::_2XX) {
        return Response::_4XX;
    }
    if (loc.root.empty() && loc.redirectionUrl.empty()) {
        return Response::_4XX;
    }
    if (!loc.methods[method]) {
        return Response::_4XX;
    }
    if(req.measure != Request::NO_BODY && maxBodySizeReq > req.body.size()){
        return Response::_4XX;
    }
    return Response::_2XX;
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
long Response::handleStartPrepingRes() {
    std::string auxTest;
    int fd = 0;
    
    if (locationPath != reconstructPath ) {
        auxTest = loc.root + reconstructPath.substr(locationPath.size(), reconstructPath.size() - locationPath.size());
    } else if(loc.defaultPath != "") {
        auxTest = loc.root + loc.defaultPath;
    }
    else{
        auxTest = loc.root;
    }
    status = PROCESSING_RES;

    std::cout << auxTest << std::endl;
    Response::statusCode responseAproxCode = filterResponseCode(auxTest, req.method, loc.defaultPath == "" ? loc.autoindex : 0);
    // TODO filter ResponseCode of checkAccess for some bad responses
    // Check access to file and non-default location

    if (responseAproxCode == Response::_2XX) {
        if(loc.autoindex == true && loc.defaultPath != "" && auxTest == loc.root)
        {
            //FILL BODY
            status = GET_RESPONSE;
            return 0;
        }

        // Check for CGI tokens in the path
        if(req.method != DELETE){
            for (std::map<std::string, std::string>::const_iterator it = loc.cgi.begin(); it != loc.cgi.end(); ++it) {
                size_t lastSlashPos = auxTest.rfind('/');
                std::string file = auxTest.substr(lastSlashPos + 1, auxTest.size());
                if (file.find(it->first) != std::string::npos) {
                    req.target.push_back(file);
                    cgiToken = it->first;
                    break;
                }
            }
        }
        // Create CGI handler if CGI token found
        if (!cgiToken.empty()) {
            newCgi = new CgiHandler(loc, cgiToken, port, req, req.target, req.queryParams);
            status = WAITING_FOR_CGI;
        }
        else {
            if(req.method == GET){
                fd = open(auxTest.c_str(), O_RDONLY);
            }
            else{
                fd = open(auxTest.c_str(), O_WRONLY);
            }
            if (fd == -1) {
                statusCodeVar = Response::_4XX;
                status = ERROR_RESPONSE;
                return 0;
            }
        }
    } else {
        /*responseAproxCode*/
        statusCodeVar = Response::_4XX;
        status = ERROR_RESPONSE;
    }

    return (long)fd;
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
long Response::handleWaitingForCgi() {
    long fdRet = newCgi->handleCgiEvent();
    if (fdRet > 0) 
        return fdRet;
    status = GET_RESPONSE;
    if(fdRet == -1){ //TODO internal server error
        statusCodeVar = Response::_4XX;
        status = ERROR_RESPONSE;
    }
    return 0;
}

/**
 * @brief Handles retrieving the response after CGI processing.
 *
 * This function retrieves the CGI response body if a CGI token was found.
 * It also prints the body to standard output and deletes the CGI handler object.
 * Further implementation should handle the remaining response build tasks.
 */
void Response::handleGetResponse() {
    std::cout << "END PROCESS" << std::endl;
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
long Response::handleBadResponse() {

    // TODO: Add checking of default pages here and statusCodeVar
    httpResponse = "HTTP/1.1 404 OK\r\nContent-Length: 0\r\nConnection: keep-alive\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n";
    status = GET_RESPONSE;
    return 0;
}
