
#include "webserv.hpp"

Server::Server(unsigned long serverID): maxBodySize(std::numeric_limits<unsigned long>::max()), serverID(serverID), port(0) {
    serverName = "serverN:" + toString(serverID); 
}

void Server::addErrorPage(int statusCode, const std::string& htmlRoute) {
    ErrPages[statusCode] = htmlRoute;
}

void Server::addLocation(const std::string& path, const Location& location){
    routes[path] = location;
}
