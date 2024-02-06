#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <cstdint>
#include <algorithm>
#include <cstddef>
#include <limits>
#include <sstream>
#include <fstream>

#define DEFAULT_CONFIG_ParserFile "test/nginxTesting/conf/1.conf"
#define METHODS_NUM 3

enum methodsEnum{
    GET,
    POST,
    DELETE
};

enum ConfigType {
    SERVER,
    SERVER_NAME,
    LOCATION,
    REDIRECT,
    ERROR_PAGE,
    PORT,
    ROOT,
    INDEX,
    METHODS,
    AUTO_INDEX,
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

        void addErrorPage(const std::string& statusCode, const std::string& htmlRoute);

        void addLocation(const std::string& path, const Location& location);

        long getNumRoutes();

        const std::map<std::string, Location> getLocations(void) const;
        
        std::string getPageStatus(const std::string& statusCode) const;

        std::vector<std::string> getKeysRoutes() const;

        const std::map<std::string, Location>& getRoutes() const;

        const std::map<std::string, std::string>& getErrPages() const;

        bool existsLocationByRoute(const std::string& path) const;
    
    private:
        //ATRIBUTES------------------------------------------------------------------·#
        //Key:Status | Value:Route_to_html
        std::map<std::string, std::string> ErrPages;
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

        void fillServers();
        
        void printServersByPort(unsigned long targetPort);
    
        std::vector<std::string> getRoutesKeysByPort(unsigned long port) const; 

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
