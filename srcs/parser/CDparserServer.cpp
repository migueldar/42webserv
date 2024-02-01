
#include "webserv.hpp"

Server::Server(uint16_t serverID): maxBodySize(std::numeric_limits<uint32_t>::max()), serverID(0), port(0) {
    (void)serverID;
}

void Server::addErrorPage(uint16_t statusCode, const std::string& htmlRoute) {
    ErrPages[statusCode] = htmlRoute;
}

void Server::addLocation(const std::string& path, const Location& location){
    routes[path] = location;
}
