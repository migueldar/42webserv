
#include "webserv.hpp"

Server::Server(): maxBodySize(std::numeric_limits<unsigned long>::max()){
    static unsigned long idCount = 0;
    Location initLocation;
    serverName = "serverN:" + toString(idCount++); 
}

void Server::addErrorPage(int statusCode, const std::string& htmlRoute) {
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


