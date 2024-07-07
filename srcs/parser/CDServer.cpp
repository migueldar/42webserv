#include "webserv.hpp"

Server::Server(): maxBodySize(std::numeric_limits<unsigned long>::max()), serverName("") {
}

Server::Server(const Server &other){
    *this = other;
}

Server& Server::operator=(const Server& other) {
    if (this != &other) {
        maxBodySize = other.maxBodySize;
        serverName = other.serverName;

        routes.clear();
        routes = other.getLocations();
        for (std::map<std::string, Location>::const_iterator it = routes.begin(); it != routes.end(); ++it) {
            std::cout << "Ruta: " << it->first << ", Root: " << it->second.root << std::endl;
        }

        errPages = other.errPages;
    }
    return *this;
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
    Location locationCopy(location);
    routes.insert(std::make_pair(path, locationCopy));
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
    for (std::map<std::string, Location>::const_iterator it = routes.begin(); it != routes.end(); ++it) {
        if (it->first == path) {
            return true;
        }
    }

    return false;
}

const std::map<std::string, Location>& Server::getRoutes() const {
    //std::cout << routes.at("'localhost'").root << std::endl;
    return routes;
}

const std::map<std::string, std::string>& Server::getErrPages() const {
    return errPages;
}


//handle route doesnt exist
const Location& Server::getLocation( std::string locationName) const {
    std::map<std::string, Location>::const_iterator it = routes.find(locationName);

    if (it != routes.end()) {
        const Location& ret = it->second;
        
        return ret;
    } else {
        static const Location defaultLocation;
        return defaultLocation;
    }
}
std::string Server::getErrPage(std::string err) const
{
	std::string errnum = err.substr(0, err.find(" "));

	return errPages.at(errnum);
}
