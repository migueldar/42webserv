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
	statusCodeVar(Response::_200), status(START_PREPING_RES), loc(getLocationByRoute(server)),\
	server(server), newCgi(NULL) {secFd.fd = 0;}

Response::~Response() {
	if (newCgi)
		delete newCgi;
}

const Location& Response::getLocationByRoute(const Server& server) {
	static Location locDef;

    if (req.measure != Request::NO_BODY && server.maxBodySize < req.body.length()) {
		statusCodeVar = _413;
		status = ERROR_RESPONSE;
		return locDef;
	}

	if(reconstructPath[reconstructPath.size() - 1] == '/')
		reconstructPath = reconstructPath.substr(0, reconstructPath.size() - 1);		
    std::string remainingPath = reconstructPath;
    size_t lastSlashPos = remainingPath.length();
    while (42) {
        std::cout << "Remaining path: " << remainingPath << std::endl;
		if (server.existsLocationByRoute(remainingPath)){
			const Location *loc = &server.getLocation(remainingPath);
			this->locationPath = remainingPath;
			if (loc->redirectionUrl != "") {
				statusCodeVar = _308;
				status = ERROR_RESPONSE;
				reconstructPath = loc->redirectionUrl + reconstructPath.substr(lastSlashPos, reconstructPath.length());
			}
			return *loc;
		}

		if(remainingPath == "")
			break;

		lastSlashPos = remainingPath.rfind('/');
        remainingPath = remainingPath.substr(0, lastSlashPos);

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
        handleWaitingForCgi(err == 2);
		if (secFd.fd != 0)
            return secFd;
    } else if (status == PROCESSING_RES) {
		handleProcessingRes();
        status = GET_RESPONSE;
    } else if (status == GET_AUTO_INDEX) {
        handleGetAutoIndex();
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
	size_t lastSlashPos = localFilePath.rfind('/');
	std::string file = localFilePath.substr(lastSlashPos + 1, localFilePath.size());

	for (std::map<std::string, std::string>::const_iterator it = loc.cgi.begin(); it != loc.cgi.end(); ++it) {
		if (file.find(it->first) != std::string::npos) {
			//TODO joan check this
			//req.target.push_back(file);
			cgiToken = it->first;
			return true;
		}
	}
    return false;
}

Response::statusCode Response::checkAccess(std::string path, enum methodsEnum method, bool autoIndex, bool& isDir) {
    struct stat fileStat;

	switch (method) {
		case GET:
			if (stat(path.c_str(), &fileStat) != 0)
				return Response::_404;
			if (S_ISDIR(fileStat.st_mode) && !autoIndex)
				return Response::_404;
			checkCgiTokens(path);
			if (S_ISDIR(fileStat.st_mode))  
				isDir = true;
			else if (!S_ISREG(fileStat.st_mode))
				return Response::_403;
			if (access(path.c_str(), R_OK) == 0)
				return Response::_200;
			return Response::_404;
		case POST:
			if (checkCgiTokens(path)) {
				if (access(path.c_str(), R_OK) == 0)
					return Response::_200;
				return Response::_404;
			}
			if (loc.uploadPath == "")
				return Response::_403;
			uploadFilePath = loc.root + loc.uploadPath;
			if (stat(uploadFilePath.c_str(), &fileStat) != 0 || !S_ISDIR(fileStat.st_mode) || access(uploadFilePath.c_str(), W_OK) != 0 || access(uploadFilePath.c_str(), X_OK) != 0)
				return Response::_404;
			if (req.headers.count("Content-Type") == 0 || req.headers.at("Content-Type").find("multipart/form-data") == std::string::npos)
				return Response::_400;
			return Response::_201;
		case DELETE:
			if (stat(path.c_str(), &fileStat) != 0)
				return Response::_404;
			if (!S_ISREG(fileStat.st_mode))
				return Response::_404;
			if (access(path.c_str(), W_OK) == 0)
				return Response::_200;
			return Response::_404;
	}
	//safeguard
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

	std::cout << "location path: " << locationPath << std::endl;
	std::cout << "reconstruct path: " << reconstructPath << std::endl;

    // Getting final path for searching in local machine
	if (locationPath != reconstructPath) {
        localFilePath = loc.root + reconstructPath.substr(locationPath.size() + 1);
    } else if (loc.defaultPath != "") {
        localFilePath = loc.root + loc.defaultPath;
    } else {
        localFilePath = loc.root;
    }

    std::cout << "localfilePath: " << localFilePath << std::endl;
	//quiza si el metodo es post ni hay que comprobar nada, o hay comportamiento distinto dependiendo de si va al cgi o no
	//o si el archivo existe o no

	bool isDir = false;
	statusCodeVar = checkAccess(localFilePath, req.method, loc.defaultPath == "" ? loc.autoindex : 0, isDir);
    // TODO Check access to file and non-default location
	if (statusCodeVar != Response::_200 && statusCodeVar != Response::_201) {
		status = ERROR_RESPONSE;
		return ;
	}

	if (loc.autoindex == true && req.method == GET && isDir) {
		status = GET_AUTO_INDEX;
	} else if (!cgiToken.empty()) {
		req.body += "EOF";
		newCgi = new CgiHandler(loc, cgiToken, port, req, req.target, req.queryParams);
		status = WAITING_FOR_CGI;
	} else {
		status = PROCESSING_RES;
		if (req.method == GET) {
			secFd.fd = open(localFilePath.c_str(), O_RDONLY);
			secFd.rw = 0;
			if (secFd.fd == -1) {
				status = ERROR_RESPONSE;
				statusCodeVar = Response::_500;
				secFd.fd = 0;
			}
		} else if (req.method == POST) {
			std::string fileName;
			try {
				fileContent = parseMultipart(fileName, req.body);
			} catch (std::exception& _) {
				status = ERROR_RESPONSE;
				statusCodeVar = Response::_400;
				secFd.fd = 0;
				return ;
			}
			secFd.fd = open((uploadFilePath + "/" + fileName).c_str(), O_TRUNC | O_WRONLY | O_CREAT, 0644);
			secFd.rw = 1;
			if (secFd.fd == -1) {
				status = ERROR_RESPONSE;
				statusCodeVar = Response::_403;
				secFd.fd = 0;
			}
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
void Response::handleWaitingForCgi(int err) {
	long fdRet = newCgi->handleCgiEvent(err);
    if (fdRet > 0) {
		secFd.fd = (int)fdRet;
		secFd.rw = fdRet >> 32;
        return ;
	}
   
	if(fdRet == -1) {
        statusCodeVar = Response::_500;
        status = ERROR_RESPONSE;
    } else {
		body += newCgi->getCgiResponse();
		status = GET_RESPONSE;
	}

	delete newCgi;
	newCgi = NULL;
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
        statusCodeVar = _500;
		status = ERROR_RESPONSE;
    }

    streamBody << "</ul></body></html>";
    body += streamBody.str();
    status = GET_RESPONSE;
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
			{
				long written = writeFile(secFd.fd, fileContent);
				if (written < 0) {
					status = ERROR_RESPONSE;
					statusCodeVar = _500;
				}
			}
			break;
		case DELETE:
			if (remove(localFilePath.c_str()) != 0) {
        		status = ERROR_RESPONSE;
				statusCodeVar = _500;
			}
			else
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
	std::string connectionState = "close";

	if (req.headers.count("Connection") == 1 && toLower(req.headers.at("Connection")) == "keep-alive")
		connectionState = "keep-alive";
	if (statusCodeVar == Response::_200) {
		httpResponse += "HTTP/1.1 200 OK\r\nContent-Length: " + toString(body.length()) + "\r\nConnection: " + connectionState + "\r\n\r\n";
		httpResponse += body;
	}
	else if (statusCodeVar == Response::_201)
		httpResponse += "HTTP/1.1 201 Created\r\nContent-Length: 0\r\nConnection: " + connectionState + "\r\n\r\n";//i think we should return location to created source
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
		case _400:
			err = "400 Bad Request";
			break;
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
