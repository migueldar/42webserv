#include <fcntl.h>
#include <sstream>
#include <dirent.h>
#include <sys/types.h>
#include "webserv.hpp"

std::string Response::getHttpResponse(){
    return httpResponse;
}

// TODO: Store server for errorPages, isnt stored now becouse isnt needed
Response::Response(std::string port, const Server& server, Request req): header(""), body(""),\
	httpResponse(""), reconstructPath(req.targetString), locationPath(""), cgiToken(""), port(port),\
	statusCodeVar(Response::_200), status(START_PREPING_RES), loc(getLocationByRoute(reconstructPath, server)),\
	server(server), newCgi(NULL), req(req) {secFd.fd = 0;}

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

Response::~Response() {}

const Location& Response::getLocationByRoute(std::string enterPath, const Server& server) {
	static Location locDef;

    if (req.measure != Request::NO_BODY && server.maxBodySize < req.body.size()) {
		statusCodeVar = _413;
		status = ERROR_RESPONSE;
		return locDef;
	}

    std::string remainingPath = enterPath;
    size_t lastSlashPos = remainingPath.size();
    bool token = 0;
    while (42) {
        // BREAKING PATHS BY '/'
        try {
            if (server.existsLocationByRoute(remainingPath)){
                const Location *loc = &server.getLocation(remainingPath);
                this->locationPath = std::string(remainingPath);
                if(loc->redirectionUrl != "") {
                    statusCodeVar = _308;
					status = ERROR_RESPONSE;
                }
                return *loc;
            }
        } catch (const std::out_of_range&) {
            std::cout << "NOT FOUND LOCATION " << std::endl; 
			//JUST TO CATH WHEN GET LOCATION STD::MAP "AT" METHOD DOESNT FIND REQUESTED LINE 
        }
        if(!token)
            token = 1;
        else
            token = 0;

		if(remainingPath == "/")
			break;

        std::cout << "Remaining path: " << remainingPath << std::endl;
		lastSlashPos = remainingPath.rfind('/');
        remainingPath = remainingPath.substr(0, lastSlashPos + token);
    }
	statusCodeVar = _404;
	status = ERROR_RESPONSE;
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
SecondaryFd Response::prepareResponse(int err) {
	(void) server;
    if (err) {
        statusCodeVar = Response::_500;
        status = ERROR_RESPONSE;
    }

	std::cout << statusCodeVar << std::endl;
    if (status == START_PREPING_RES) {
        handleStartPrepingRes();
        if (secFd.fd != 0)
            return secFd;
    }
    
    if (status == WAITING_FOR_CGI) {
        // ret = handleWaitingForCgi();

        // if (ret != 0)
        //     return ret;
    } else if (status == PROCESSING_RES) {
		handleProcessingRes();
        status = GET_RESPONSE;
    } else if (status == GET_AUTO_INDEX) {
        handleGetAutoIndex();
        status = GET_RESPONSE;
    }

    if (status == GET_RESPONSE) {
        handleGetResponse();
    } else if (status == ERROR_RESPONSE)
		handleBadResponse();

	secFd.fd = -1;
	return secFd;
}

// TODO change parameters to be more specific
bool Response::checkCgiTokens(const std::string &localFilePath) {
    if (req.method != DELETE) {
        for (std::map<std::string, std::string>::const_iterator it = loc.cgi.begin(); it != loc.cgi.end(); ++it) {
            size_t lastSlashPos = localFilePath.rfind('/');
            std::string file = localFilePath.substr(lastSlashPos + 1, localFilePath.size());
            if (file.find(it->first) != std::string::npos) {
                req.target.push_back(file);
                cgiToken = it->first;
                return true;
            }
        }
    }
    return false;
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
 * @return Always returns 0 to continue state machine (indicators for future implementation).
 */
void Response::handleStartPrepingRes() {
	if (!loc.methods[req.method]) {
        statusCodeVar = _405;
		return ;
    }

    // Getting final path for searching in local machine
	if (locationPath != reconstructPath) {
        localFilePath = loc.root + reconstructPath.substr(locationPath.size(), reconstructPath.size() - locationPath.size());
    } else if (loc.defaultPath != "") {
        localFilePath = loc.root + loc.defaultPath;
    } else {
        localFilePath = loc.root;
    }

    std::cout << "localfilePath: " << localFilePath << std::endl;
	//quiza si el metodo es post ni hay que comprobar nada, o hay comportamiento distinto dependiendo de si va al cgi o no
	//o si el archivo existe o no
	statusCodeVar = checkAccess(localFilePath, req.method, loc.defaultPath == "" ? loc.autoindex : 0);
    // TODO Check access to file and non-default location
	if (statusCodeVar != Response::_200) {
		status = ERROR_RESPONSE;
		return ;
	}

	if (loc.autoindex == true && req.method == GET && loc.defaultPath == "" && localFilePath == loc.root) {
		status = GET_AUTO_INDEX;
	} else if (checkCgiTokens(localFilePath)) {
		newCgi = new CgiHandler(loc, cgiToken, port, req, req.target, req.queryParams);
		status = WAITING_FOR_CGI;
	} else {
		status = PROCESSING_RES;
		if (req.method == GET) {
			secFd.fd = open(localFilePath.c_str(), O_RDONLY);
			secFd.rw = 0;
		} else {
			//TODO post, delete doesnt need fd
		}
		if (secFd.fd == -1) {
			status = ERROR_RESPONSE;
			statusCodeVar = Response::_500; //en caso de darse este error es un 500, ya que ya se ha comprobado el access
			secFd.fd = 0;
		}
	}
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
    if(fdRet == -1){
        statusCodeVar = Response::_500;
        status = ERROR_RESPONSE;
    }
    return 0;
}

long Response::handleGetAutoIndex() {
    std::ostringstream streamBody;
    streamBody << "<html><head><title>Index of " << localFilePath << "</title></head><body>";
    streamBody << "<h1>Index of " << localFilePath << "</h1>";
    streamBody << "<ul>";

    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(localFilePath.c_str())) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            std::string fileName = entry->d_name;
            if (fileName != "." && fileName != "..") {
                streamBody << "<li><a href=\"" << fileName << "\">" << fileName << "</a></li>";
            }
        }
        closedir(dir);
    } else {
        streamBody << "<li>Error: unable to open directory " << localFilePath << "</li>"; //should put error status to true maybe instead of writting the error in the html
    }

    streamBody << "</ul></body></html>";
    body = streamBody.str();
    std::cout << body << std::endl;
    return 0;
}

void Response::handleProcessingRes() {
	switch (req.method) {
		case GET: 
			try {
				body = readFile(secFd.fd);
			} catch (std::runtime_error& e) {
				status = ERROR_RESPONSE;
				statusCodeVar = _500;
			}
		case POST:
			break;
		case DELETE:
			std::cout << "handle delete" << std::endl;
			if (remove(localFilePath.c_str()) != 0) {
        		status = ERROR_RESPONSE;
				statusCodeVar = _500;
			}
			body = "<p> File: " + localFilePath + " removed</p>";
	}
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

	httpResponse = "HTTP/1.1 200 OK\r\nContent-Length: " + toString(body.size()) + "\r\nConnection: keep-alive\r\n\r\n" + body;
}

/**
 * @brief Handles the scenario where the requested file or location is not found.
 *
 * This function generates a standard HTTP 404 response indicating that the requested
 * resource could not be found. It sets the httpResponse string accordingly and updates
 * the status to GET_RESPONSE for further response handling.
 */
void Response::handleBadResponse() {
	std::string err;
	//err pages
	switch (statusCodeVar)
	{
		case _200:
			//safeguard, will never run
			break;
		case _308:
			httpResponse = "HTTP/1.1 308 Permanent Redirect\r\nLocation: " + loc.redirectionUrl + "\r\nContent-Length: 0 \r\nConnection: keep-alive\r\n\r\n";
			return;
		case _404:
			err = "404 Not Found";
			break;
		case _405:
			err = "405 Method Not Allowed";
			break;
		case _413:
			err = "413 Payload Too Large";
			break;
		case _500:
			err = "500 Internal Server Error";
			break;
	}

    httpResponse = "HTTP/1.1 " + err + "\r\nContent-Length: " + toString(err.size() + 9) + "\r\nConnection: close\r\n\r\n<h1>" + err + "</h1>";
}
