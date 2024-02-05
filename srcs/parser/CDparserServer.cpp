
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
