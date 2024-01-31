
#include "../../inc/parser.hpp"

parserServer::parserServer(uint16_t serverID): maxBodySize(std::numeric_limits<uint32_t>::max()), serverID(0), port(0) {

}

void parserServer::addErrorPage(uint16_t statusCode, const std::string& htmlRoute) {
    parserErrPages[statusCode] = htmlRoute;
}

void parserServer::addLocation(const std::string& path, const parserLocations& location){
    routes[path] = location;
}
