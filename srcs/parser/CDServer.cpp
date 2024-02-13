#include "webserv.hpp"

Server::Server(): maxBodySize(std::numeric_limits<unsigned long>::max()), serverName("") {
    Location initLocation;
}

Server::Server(std::vector<std::string>& s, std::vector<Location>& l) {
	for (size_t i = 0; i < s.size(); i++) {
		routes[s[i]] = l[i];
	}
}

std::string Server::getPageStatus(const std::string& statusCode) const {
    std::map<std::string, std::string>::const_iterator it = errPages.find(statusCode);

    if (it != errPages.end()) {
        return it->second;
    } else {
        return std::string();  
    }
}

void Server::addErrorPage(const std::string& statusCode, const std::string& htmlRoute) {
    errPages[statusCode] = htmlRoute;
}

void Server::addLocation(const std::string& path, const Location& location) {
    routes[path] = location;
}



long Server::getNumRoutes(void){
    return(routes.size());
}

const std::map<std::string, Location>& Server::getLocations(void) const{
    return(routes);
}

std::vector<std::string> Server::getKeysRoutes() const {
    std::vector<std::string> exitRoutes;

    for (std::map<std::string, Location>::const_iterator iter = routes.begin(); iter != routes.end(); ++iter) {
        exitRoutes.push_back(iter->first);
    }

    return exitRoutes;
}

bool Server::existsLocationByRoute(const std::string& path) const {
    static const Location emptyLocation; 

    std::map<std::string, Location>::const_iterator locationIter = routes.find(path);
    if (locationIter != routes.end()) {
        return true;
    } else {
        return false;
    }
}

const std::map<std::string, Location>& Server::getRoutes() const {
    return routes;
}

const std::map<std::string, std::string>& Server::getErrPages() const {
    return errPages;
}


//handle route doesnt exist
const Location& Server::getLocation(std::string locationName) const {
	return (routes.at(locationName));
}
