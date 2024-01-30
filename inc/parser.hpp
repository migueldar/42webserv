#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <vector>


class parserLocations{
    //
    std::string locationPath = "";

    //std::string rootPath;

    //NOT DEFAULT NEEDED
    std::string indexPath;
};

class parserServer {

    // FULL 4 bytes AUTOASIGNABLE
    uint32_t serverID; 

    // RANGES [0, 65535]
    uint16_t port;

    // MUST HAVE DEFAULT    
    std::string serverName;

    // ALL PATHS EITHER EXISTS OR NOT
    std::vector<parserLocations> routes;

};

class parserFile{
    uint32_t numfile; 

    std::vector<uint32_t> *prioIdServ;

    std::vector<parserServer> serverDefinitinos;
};




#endif