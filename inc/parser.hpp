#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <cstdint>
#include <limits>

#define DEFAULT_CONFIG_FILE 
#define METHODS_NUM 3

enum methodsEnum{
    GET,
    POST,
    DELETE
};

class parserLocations {
public:

    //ATRIBUTES------------------------------------------------------------------·#
    //NOT NEEDED DEFAULT
    std::string root;
    //NOT DEFAULT NEEDED
    std::string defaultPath;
    // NO DEFAULT NEEDED
    std::string redirectionUrl;
    //MANAGES ESPECIFIC RULES FOR METHODS
    bool methods[METHODS_NUM];
    //OFF predeterminado
    bool autoindex;

    //METHODS--------------------------------------------------------------------·#
    parserLocations();
};

class parserServer {
    public:

        //ATRIBUTES------------------------------------------------------------------·#
        //PARSE MAX BODYSIZE
        uint32_t maxBodySize;
        // FULL 4 bytes AUTOASIGNABLE
        uint32_t serverID;
        // MUST HAVE DEFAULT
        std::string serverName;
        // RANGES [0, std::numeric_limits<uint16_t>::max()]
        uint16_t port;
        //Key:Status | Value:Route_to_html
        std::map<uint16_t, std::string> parserErrPages;
        // Key:LocationPath | Value:classLocation 
        std::map<std::string, parserLocations> routes;


        //METHODS--------------------------------------------------------------------·#
        parserServer(uint16_t serverID);

        void addErrorPage(uint16_t statusCode, const std::string& htmlRoute);

        void addLocation(const std::string& path, const parserLocations& location);
};

class parserFile {
    public:
        FILE *configFile;

        //ATRIBUTES------------------------------------------------------------------·#
        //Auto increment counter
        uint32_t numfile;
        //Vector of priorities ids
        std::vector<uint32_t> prioIdServ;
        //Server vector
        std::vector<parserServer> serverDefinitions;


        //METHODS--------------------------------------------------------------------·#
        parserFile(std::string routeToFile);

        void addPriorityId(uint32_t priorityId);
};



#endif

enum methodsEnum {
    GET,
    POST,
    DELETE
};

