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
		ParserFile();
        ParserFile(std::string routeToParserFile);
        ~ParserFile(void);
		ParserFile &operator=(ParserFile const& rhs);

        void addServerDefinition(Server newServer);

        void fillServers();
        
        void printServersByPort(unsigned int targetPort);
    
        int checkRoutesServer(const std::map<std::string, Location>& routes, const std::string& keyToFind);

        int checkServerName(const std::vector<Server>& servers, const std::string& keyToFind);
		const std::map<unsigned int, std::vector<Server> >& getServers() const;

    private:
        //ATRIBUTES------------------------------------------------------------------·#
        //Server vector
        std::map<unsigned int, std::vector<Server> > serverDefinitions;
        
};

#endif
