#ifndef CDPARSERFILE_HPP
#define CDPARSERFILE_HPP

#include "CDServer.hpp"
#include <fstream>


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
        
        void printServersByPort(unsigned int targetPort);
    
        int checkRoutesServer(const std::map<std::string, Location>& routes, const std::string& keyToFind);

    private:
        //ATRIBUTES------------------------------------------------------------------·#
        //Server vector
        std::map<unsigned int, std::map<std::string, std::vector<Server> > > serverDefinitions;
        
};

#endif