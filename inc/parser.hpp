#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <cstddef>
#include <limits>
#include <sstream>
#include <fstream>

#define DEFAULT_CONFIG_ParserFile "test/nginxTesting/conf/nginx2.conf"
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
        unsigned long maxBodySize;
        // FULL 4 bytes AUTOASIGNABLE
        std::string serverName;        

        //METHODS--------------------------------------------------------------------·#
        Server();

        void addErrorPage(int statusCode, const std::string& htmlRoute);

        void addLocation(const std::string& path, const Location& location);
    
    private:
        //ATRIBUTES------------------------------------------------------------------·#
        //Key:Status | Value:Route_to_html
        std::map<int, std::string> ErrPages;
        // Key:LocationPath | Value:classLocation 
        std::map<std::string, Location> routes;
};

//src/paser/CDParserFile.cpp
class ParserFile {
    public:
        //FILE TO MANAGE
        std::ifstream configParserFile;

        //ATRIBUTES------------------------------------------------------------------·#

        //METHODS--------------------------------------------------------------------·#
        ParserFile(std::string routeToParserFile);
        ~ParserFile(void);

        void addServerDefinition(Server newServer);

        long fillServer(std::map<std::string, ConfigType> &configTypeMap);
        
        void parseFile(void);

        void printServersByPort(unsigned long targetPort);
    private:
        //ATRIBUTES------------------------------------------------------------------·#
        //Server vector
        std::map<unsigned long, std::vector<Server> > serverDefinitions;
        
};

//UTILS
std::vector<std::string> splitString(const std::string& input, char delimiter);
unsigned long stringToUnsignedLong(const std::string& str);

template <typename T>
std::string toString(T value) {
    std::ostringstream os;
    os << value;
    return os.str();
}

#endif
