#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <cstdint>
#include <limits>
#include <fstream>

#define DEFAULT_CONFIG_ParserFile "test/nginxTesting/conf/nginx0.conf"
#define METHODS_NUM 3

enum methodsEnum{
    GET,
    POST,
    DELETE
};

enum ConfigType {
    SERVER_NAME,
    LOCATION,
    REDIRECT,
    ERROR_PAGE,
    PORT,
    ROOT,
    INDEX,
    METHODS,
    BRACE_CLOSE,
    UNKNOWN
};


//src/paser/CDLocation.cpp
class Location {
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
    Location();
};

//src/paser/CDServer.cpp
class Server {
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

        //METHODS--------------------------------------------------------------------·#
        Server(uint16_t serverID);

        void addErrorPage(uint16_t statusCode, const std::string& htmlRoute);

        void addLocation(const std::string& path, const Location& location);
    
    private:
        //ATRIBUTES------------------------------------------------------------------·#
        //Key:Status | Value:Route_to_html
        std::map<uint16_t, std::string> ErrPages;
        // Key:LocationPath | Value:classLocation 
        std::map<std::string, Location> routes;
};

//src/paser/CDParserFile.cpp
class ParserFile {
    public:
        //FILE TO MANAGE
        std::ifstream configParserFile;

        //ATRIBUTES------------------------------------------------------------------·#
        //Auto increment counter
        uint32_t numParserFile;

        //METHODS--------------------------------------------------------------------·#
        ParserFile(std::string routeToParserFile);
        ~ParserFile(void);

        void addPriorityId(uint32_t priorityId);

        void addServerDefinition(Server newServer);
        
        int32_t fillServer(void);
        
        void parseFile(void);
    private:
        //ATRIBUTES------------------------------------------------------------------·#
        //Vector of priorities ids
        std::vector<uint32_t> prioIdServ;
        //Server vector
        std::vector<Server> serverDefinitions;

};

//UTILS
std::vector<std::string> splitString(const std::string& input, char delimiter);

#endif
