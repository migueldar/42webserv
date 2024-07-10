#include <fcntl.h>
#include <sstream>
#include <dirent.h>
#include <sys/types.h>
#include "webserv.hpp"

std::string Response::getPartHttpResponse() {
    return httpResponse.popFirst();
}

bool Response::done() {
	return httpResponse.empty();
}

Response::Response(std::string port, const Server& server, Request& req): req(req), header(""),\
	reconstructPath(req.targetString), locationPath(""), cgiToken(""), port(port),\
	statusCodeVar(Response::_200), status(START_PREPING_RES), loc(getLocationByRoute(reconstructPath, server)),\
	server(server), newCgi(NULL) {secFd.fd = 0;}

Response::~Response() {}

const Location& Response::getLocationByRoute(std::string enterPath, const Server& server) {
	static Location locDef;

    if (req.measure != Request::NO_BODY && server.maxBodySize < req.body.length()) {
		statusCodeVar = _413;
		status = ERROR_RESPONSE;
		return locDef;
	}

    std::string remainingPath = enterPath;
    size_t lastSlashPos = remainingPath.size();
    bool token = 0;
    while (42) {
		if (server.existsLocationByRoute(remainingPath)){
			const Location *loc = &server.getLocation(remainingPath);
			this->locationPath = std::string(remainingPath);
			if(loc->redirectionUrl != "") {
				statusCodeVar = _308;
				status = ERROR_RESPONSE;
				token = 0;
				reconstructPath = loc->redirectionUrl + enterPath.substr(lastSlashPos + !token, enterPath.length());
			}
			return *loc;
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
//1 pollerr, 2 pollhup
SecondaryFd Response::prepareResponse(int err) {
    if (err == 1) {
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
        handleWaitingForCgi();
		if (secFd.fd != 0)
            return secFd;
    } else if (status == PROCESSING_RES) {
		handleProcessingRes();
        status = GET_RESPONSE;
    } else if (status == GET_AUTO_INDEX) {
        handleGetAutoIndex();
        status = GET_RESPONSE;
    }

    if (status == GET_RESPONSE) {
        handleGetResponse();
    } else if (status == ERROR_RESPONSE) {
		handleBadResponse();
		if (secFd.fd != 0)
			return secFd;
	} else if (status == ERROR_RESPONSE_PAGE) {
		handleBadResponsePage();
	}

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

Response::statusCode Response::checkAccess(std::string path, enum methodsEnum method, bool autoIndex) {
    struct stat fileStat;

	switch (method){
		case GET:
			if (stat(path.c_str(), &fileStat) != 0)
				return Response::_404;
			if (S_ISDIR(fileStat.st_mode) && !autoIndex)
				return Response::_404;
			if (access(path.c_str(), R_OK) == 0)
				return Response::_200;
			return Response::_404;
		case POST:
			//este check hace mas cosas asi que rompe
			if (checkCgiTokens(path)) {
				if (access(path.c_str(), R_OK) == 0)
					return Response::_200;
				return Response::_404;
			}
			if (loc.uploadPath == "")
				return Response::_403;
			uploadFilePath = loc.root + loc.uploadPath;
			//have to check in the upload path, not here
			if (stat(uploadFilePath.c_str(), &fileStat) != 0 || !S_ISDIR(fileStat.st_mode))
				return Response::_404;
			uploadFilePath = path;
			if (access(uploadFilePath.c_str(), F_OK) == 0 && access(uploadFilePath.c_str(), W_OK) != 0)
                return Response::_403;
			return Response::_201;
		case DELETE:
			if (stat(path.c_str(), &fileStat) != 0)
				return Response::_404;
			if (S_ISDIR(fileStat.st_mode))
				return Response::_404;
			if (access(path.c_str(), W_OK) == 0)
				return Response::_200;
			return Response::_404;
	}
	return Response::_404;
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
		status = ERROR_RESPONSE;
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
	if (statusCodeVar != Response::_200 && statusCodeVar != Response::_201) {
		status = ERROR_RESPONSE;
		return ;
	}

	if (loc.autoindex == true && req.method == GET && loc.defaultPath == "" && localFilePath == loc.root) {
		status = GET_AUTO_INDEX;
	} else if (!cgiToken.empty()) {
		newCgi = new CgiHandler(loc, cgiToken, port, req, req.target, req.queryParams);
		status = WAITING_FOR_CGI;
	} else {
		status = PROCESSING_RES;
		if (req.method == GET) {
			secFd.fd = open(localFilePath.c_str(), O_RDONLY);
			secFd.rw = 0;
		} else if (req.method == POST) {
			secFd.fd = open(uploadFilePath.c_str(), O_APPEND | O_WRONLY | O_CREAT, 0644);
			secFd.rw = 1;
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
void Response::handleWaitingForCgi() {
	long fdRet = newCgi->handleCgiEvent();
    if (fdRet > 0) {
		secFd.fd = (int)fdRet;
		secFd.rw = fdRet >> 32;
        return ;
	}

    status = GET_RESPONSE;
    if (!cgiToken.empty()) {
        body += newCgi->getCgiResponse();
        delete newCgi;
		newCgi = NULL;
    }

	if(fdRet == -1){
        statusCodeVar = Response::_500;
        status = ERROR_RESPONSE;
    }
	secFd.fd = 0;
    return ;
}	

void Response::handleGetAutoIndex() {
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
    body += streamBody.str();
    std::cout << body << std::endl;
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
			break;
		case POST:
			if(cgiToken == ""){
				long written = writeFile(secFd.fd, req.body);
				if (written < 0) {
					status = ERROR_RESPONSE;
					statusCodeVar = _500;
				}
			}
			break;
		case DELETE:
			std::cout << "handle delete" << std::endl;
			if (remove(localFilePath.c_str()) != 0) {
        		status = ERROR_RESPONSE;
				statusCodeVar = _500;
			}
			body += "<p> File: " + localFilePath + " removed</p>";
			break;
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
	if (statusCodeVar == Response::_200) {
		httpResponse += "HTTP/1.1 200 OK\r\nContent-Length: " + toString(body.length()) + "\r\nConnection: keep-alive\r\n\r\n";
		httpResponse += body;
	}
	else if (statusCodeVar == Response::_201)
		httpResponse += "HTTP/1.1 201 Created\r\nContent-Length: 0\r\nConnection: keep-alive\r\nLocation: " + uploadFilePath.substr(loc.root.length() - 1) + "\r\n\r\n";
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

	secFd.fd = 0;
	switch (statusCodeVar)
	{
		case _200:
			//safeguard, will never run
			return;
		case _201:
			//safeguard, will never run
			return;
		case _308:
			httpResponse += "HTTP/1.1 308 Permanent Redirect\r\nLocation: " + reconstructPath + "\r\nContent-Length: 0\r\nConnection: keep-alive\r\n\r\n";
			return;
		case _403:
			err = "403 Forbidden";
			break;
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

	try {
		std::string errPage = server.getErrPage(err);
		secFd.fd = open(errPage.c_str(), O_RDONLY);
		if (secFd.fd > 0)
		{
			httpResponse += "HTTP/1.1 " + err + "\r\nConnection: close\r\nContent-Length: ";
			status = ERROR_RESPONSE_PAGE;
			return ;
		}
		else
			err = "500 Internal Server Error";
	}
	catch (std::exception& _) {}

    httpResponse += "HTTP/1.1 " + err + "\r\nContent-Length: " + toString(err.size() + 9) + "\r\nConnection: close\r\n\r\n<h1>" + err + "</h1>";
}

void Response::handleBadResponsePage() {
	std::string err;

	try {
		stringWrap page = readFile(secFd.fd);
		httpResponse += toString(page.length()) + "\r\n\r\n";
		httpResponse += page;
		return ;
	}
	catch (std::exception& _) {
		err = "500 Internal Server Error";
	}
    httpResponse += "HTTP/1.1 " + err + "\r\nContent-Length: " + toString(err.size() + 9) + "\r\nConnection: close\r\n\r\n<h1>" + err + "</h1>";
}
