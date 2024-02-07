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
        
        void printServersByPort(unsigned long targetPort);
    
        std::vector<std::string> getRoutesKeysByPort(unsigned long port) const; 

    private:
        //ATRIBUTES------------------------------------------------------------------·#
        //Server vector
        std::map<unsigned long, std::vector<Server> > serverDefinitions;
        
};

#endif