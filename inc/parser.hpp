#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <vector>
#include <cstdint>
#include <limits>

#define METHODS_NUM 3

enum methodsEnum{
    GET = 0;
    POST;
    DELETE;
}

class parserLocations{
    std::string  root;

    //NOT DEFAULT NEEDED
    std::string indexPath;

    // NO DEFAULT NEEDED 
    std::string redirectionUrl = "";

    //MANAGES ESPECIFIC RULES FOR METHODS
    bool methods[METHODS_NUM];

    //OFF predeterminado
    bool autoindex;

};


class parserServer {
    //PARSE MAX BODYSIZE
    uint32_t maxBodySize;

    // FULL 4 bytes AUTOASIGNABLE
    uint32_t serverID; 

    // MUST HAVE DEFAULT    
    std::string serverName;

    // RANGES [0, 65535]
    uint16_t port;

    //Key:Status | Value:Route_to_html
    std::map<uint16_t, std::string> parserErrPages;

    // Key:LocationPath | Value:classLocation 
    std::map<std::string, parserLocations> routes;

};

class parserFile{
    //Auto increment counter
    uint32_t numfile; 

    //Vector of priorities ids
    std::vector<uint32_t> prioIdServ;
    
    //Server vector
    std::vector<parserServer> serverDefinitinos;
};




#endif