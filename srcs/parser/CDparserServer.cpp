
#include "webserv.hpp"

Server::Server(): maxBodySize(std::numeric_limits<unsigned long>::max()){
    static unsigned long idCount = 0;
    Location initLocation;
    serverName = "serverN:" + toString(idCount++); 
}

std::string Server::getPageStatus(const std::string& statusCode) const {
    std::map<std::string, std::string>::const_iterator it = ErrPages.find(statusCode);

    if (it != ErrPages.end()) {
        return it->second;
    } else {
        return std::string();  
    }
}

void Server::addErrorPage(const std::string& statusCode, const std::string& htmlRoute) {
    ErrPages[statusCode] = htmlRoute;
}

void Server::addLocation(const std::string& path, const Location& location){
    routes[path] = location;
}


long Server::getNumRoutes(void){
    return(routes.size());
}

const std::map<std::string, Location> Server::getLocations(void) const{
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
    return ErrPages;
}